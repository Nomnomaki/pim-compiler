#pragma once

#include <cstdint>
#include <string> // For helper function
#include <sstream> // For helper function
#include <iomanip> // For helper function

namespace pim {

// Redefine Opcode values to fit in 2 bits
enum class Opcode : uint32_t {
    MEM_LOAD      = 0, // 0b00
    MEM_STORE     = 1, // 0b01
    COMPUTE_SETUP = 2, // 0b10
    COMPUTE_EXEC  = 3  // 0b11
};

// Define bit positions and masks based on the image (total 19 bits)
namespace instr_format {
    constexpr uint32_t OPCODE_SHIFT = 17;
    constexpr uint32_t OPCODE_MASK = 0x3; // 2 bits

    constexpr uint32_t CORE_PTR_SHIFT = 11;
    constexpr uint32_t CORE_PTR_MASK = 0x3F; // 6 bits

    constexpr uint32_t RD_FLAG_SHIFT = 10;
    constexpr uint32_t RD_FLAG_MASK = 0x1; // 1 bit

    constexpr uint32_t WR_FLAG_SHIFT = 9;
    constexpr uint32_t WR_FLAG_MASK = 0x1; // 1 bit

    constexpr uint32_t ROW_ADDR_SHIFT = 0;
    constexpr uint32_t ROW_ADDR_MASK = 0x1FF; // 9 bits

    // Helper function to pack fields into a 32-bit instruction word
    inline uint32_t pack(Opcode op, uint32_t core_ptr, bool rd, bool wr, uint32_t row_addr) {
        uint32_t instruction = 0;
        instruction |= (static_cast<uint32_t>(op) & OPCODE_MASK) << OPCODE_SHIFT;
        instruction |= (core_ptr & CORE_PTR_MASK) << CORE_PTR_SHIFT;
        instruction |= (rd ? 1U : 0U) << RD_FLAG_SHIFT; // Ensure rd is 0 or 1 shifted
        instruction |= (wr ? 1U : 0U) << WR_FLAG_SHIFT; // Ensure wr is 0 or 1 shifted
        instruction |= (row_addr & ROW_ADDR_MASK) << ROW_ADDR_SHIFT;
        return instruction;
    }

    // Helper structure/functions to unpack for display (optional but useful)
    struct UnpackedInstr {
        Opcode opcode;
        uint32_t core_ptr;
        bool rd;
        bool wr;
        uint32_t row_addr;
    };

    inline UnpackedInstr unpack(uint32_t instruction) {
        UnpackedInstr unpacked;
        unpacked.opcode   = static_cast<Opcode>((instruction >> OPCODE_SHIFT) & OPCODE_MASK);
        unpacked.core_ptr = (instruction >> CORE_PTR_SHIFT) & CORE_PTR_MASK;
        unpacked.rd       = ((instruction >> RD_FLAG_SHIFT) & RD_FLAG_MASK) != 0;
        unpacked.wr       = ((instruction >> WR_FLAG_SHIFT) & WR_FLAG_MASK) != 0;
        unpacked.row_addr = (instruction >> ROW_ADDR_SHIFT) & ROW_ADDR_MASK;
        return unpacked;
    }

     // Helper to get Opcode as string for printing
     inline std::string opcode_to_string(Opcode op) {
         switch (op) {
             case Opcode::MEM_LOAD: return "MEM_LOAD";
             case Opcode::MEM_STORE: return "MEM_STORE";
             case Opcode::COMPUTE_SETUP: return "COMPUTE_SETUP";
             case Opcode::COMPUTE_EXEC: return "COMPUTE_EXEC";
             default: return "UNKNOWN";
         }
     }

} // namespace instr_format

// Typedef for clarity in the rest of the code
using InstructionWord = uint32_t;

} // namespace pim 