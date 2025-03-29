#include "compiler.hpp"
#include <stdexcept>
#include <vector>
#include <iostream>

namespace pim {

// Main compilation function: Generates IR first, then translates to PIM instructions
std::vector<InstructionWord> Compiler::compile_matrix_mult(
    const std::vector<std::vector<int>>& matrix_a,
    const std::vector<std::vector<int>>& matrix_b)
{
    if (matrix_a.empty() || matrix_b.empty() || matrix_a[0].empty() || matrix_b[0].empty()) {
        // Consider throwing an exception or returning an empty vector
        // For simplicity, we'll assume valid inputs based on previous context
         throw std::runtime_error("Input matrices cannot be empty.");
    }

    size_t rows_a = matrix_a.size();
    size_t cols_a = matrix_a[0].size(); // Also rows_b
    size_t rows_b = matrix_b.size();
    size_t cols_b = matrix_b[0].size();

    if (cols_a != rows_b) {
         throw std::runtime_error("Matrix dimensions mismatch: cols_a must equal rows_b.");
    }

    // --- Stage 1: Generate Intermediate Representation (IR) ---
    std::vector<IROperation> ir_code;
    // Estimate size to potentially reduce reallocations
    ir_code.reserve(rows_a * cols_b * (1 + cols_a * 3 + 1));

    for (size_t i = 0; i < rows_a; ++i) {
        for (size_t j = 0; j < cols_b; ++j) {
            // Reset accumulator for calculating C[i][j]
            ir_code.emplace_back(IROpType::RESET_ACC, i, j);

            for (size_t k = 0; k < cols_a; ++k) { // cols_a is the inner dimension
                // Load A[i][k] into buffer 0
                ir_code.emplace_back(IROpType::LOAD_A_ELEMENT, i, k, 0);
                // Load B[k][j] into buffer 1
                ir_code.emplace_back(IROpType::LOAD_B_ELEMENT, k, j, 1);
                // Execute MAC (Accumulator += Buffer0 * Buffer1)
                ir_code.emplace_back(IROpType::EXECUTE_MAC); // Use explicit constructor
            }
            // Store result from Accumulator to C[i][j]
            ir_code.emplace_back(IROpType::STORE_C_ELEMENT, i, j);
        }
    }

    // --- Stage 2: Translate IR to PIM Instructions (Packed uint32_t) ---
    return translate_ir_to_pim(ir_code, rows_a, cols_a, cols_b);
}

// Private helper function: Translates IR sequence to PIM Instructions
// This acts as the "lookup table" or translator
std::vector<InstructionWord> Compiler::translate_ir_to_pim(
    const std::vector<IROperation>& ir_code,
    size_t rows_a,
    size_t cols_a,
    size_t cols_b)
{
    std::vector<InstructionWord> instructions;
    instructions.reserve(ir_code.size());

    for (const auto& ir_op : ir_code) {
        Opcode opcode_val;
        uint32_t core_ptr_val = 0; // Default core_ptr
        bool rd_flag = false;
        bool wr_flag = false;
        uint32_t row_addr_val = 0;

        switch (ir_op.type) {
            case IROpType::RESET_ACC:
                opcode_val = Opcode::COMPUTE_SETUP;
                // Flags and address remain 0
                break;

            case IROpType::LOAD_A_ELEMENT:
                opcode_val = Opcode::MEM_LOAD;
                rd_flag = true;
                wr_flag = false;
                // Use target_buffer as core_ptr for loads (fits in 6 bits)
                core_ptr_val = static_cast<uint32_t>(ir_op.target_buffer);
                // Calculate address and MASK to 9 bits
                row_addr_val = (MATRIX_A_BASE + ir_op.i * cols_a + ir_op.k);
                break;

            case IROpType::LOAD_B_ELEMENT:
                opcode_val = Opcode::MEM_LOAD;
                rd_flag = true;
                wr_flag = false;
                 // Use target_buffer as core_ptr for loads (fits in 6 bits)
                core_ptr_val = static_cast<uint32_t>(ir_op.target_buffer);
                 // Calculate address and MASK to 9 bits
                row_addr_val = (MATRIX_B_BASE + ir_op.k * cols_b + ir_op.j);
                break;

            case IROpType::EXECUTE_MAC:
                opcode_val = Opcode::COMPUTE_EXEC;
                 // Flags and address remain 0
                break;

            case IROpType::STORE_C_ELEMENT:
                opcode_val = Opcode::MEM_STORE;
                rd_flag = false;
                wr_flag = true;
                 // Use default core_ptr_val (0) for stores
                 // Calculate address and MASK to 9 bits
                row_addr_val = (MATRIX_C_BASE + ir_op.i * cols_b + ir_op.j);
                break;

            default:
                 std::cerr << "Warning: Unknown IR operation type encountered during translation: "
                           << static_cast<int>(ir_op.type) << ". Skipping." << std::endl;
                 continue; // Skip this IR operation
        }

        // Pack the fields into a single instruction word
        instructions.push_back(
            instr_format::pack(opcode_val, core_ptr_val, rd_flag, wr_flag, row_addr_val)
        );
    }

    return instructions;
}

// --- Removed old helper functions as they are no longer used ---
// std::vector<Instruction> Compiler::generate_lut_instructions() { ... }
// std::vector<Instruction> Compiler::generate_load_instructions(...) { ... }
// std::vector<Instruction> Compiler::generate_compute_instructions(...) { ... }
// std::vector<Instruction> Compiler::generate_store_instructions(...) { ... }

} // namespace pim 