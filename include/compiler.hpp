#pragma once

#include <vector>
#include <string>
#include "instruction.hpp"
#include "ir.hpp"
#include <cstdint>

namespace pim {

// Base addresses for matrices in simulated PIM memory
constexpr uint32_t MATRIX_A_BASE = 0;
constexpr uint32_t MATRIX_B_BASE = 100; // Example: Allocate space after A
constexpr uint32_t MATRIX_C_BASE = 200; // Example: Allocate space after B

class Compiler {
public:
    Compiler() = default;

    // Compile matrix multiplication code into PIM instructions
    std::vector<InstructionWord> compile_matrix_mult(
        const std::vector<std::vector<int>>& matrix_a,
        const std::vector<std::vector<int>>& matrix_b
    );

private:
    // Translates the Intermediate Representation (IR) code to PIM Instructions
    std::vector<InstructionWord> translate_ir_to_pim(
        const std::vector<IROperation>& ir_code,
        size_t rows_a,
        size_t cols_a,
        size_t cols_b);

    // Helper methods
    std::vector<InstructionWord> generate_lut_instructions();
    std::vector<InstructionWord> generate_load_instructions(
        const std::vector<std::vector<int>>& matrix,
        uint32_t base_addr
    );
    std::vector<InstructionWord> generate_compute_instructions(
        uint32_t rows_a,
        uint32_t cols_a,
        uint32_t cols_b
    );
    std::vector<InstructionWord> generate_store_instructions(
        uint32_t rows,
        uint32_t cols,
        uint32_t base_addr
    );
};

} // namespace pim 