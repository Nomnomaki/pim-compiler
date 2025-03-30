#pragma once

#include <cstdint>
#include <vector>
#include <type_traits> // Needed for std::enable_if_t (though we removed its use)

namespace pim {

// Intermediate Representation Operation Types
enum class IROpType {
    RESET_ACC,      // Reset the accumulator for a new C[i][j] element
    LOAD_A_ELEMENT, // Load an element from Matrix A
    LOAD_B_ELEMENT, // Load an element from Matrix B
    EXECUTE_MAC,    // Perform the Multiply-Accumulate operation
    STORE_C_ELEMENT // Store the result from Accumulator to Matrix C
};

// Intermediate Representation Operation Structure
struct IROperation {
    IROpType type;

    // Operands (relevant fields depend on type)
    size_t i = 0; // Row index for A or C
    size_t j = 0; // Col index for B or C
    size_t k = 0; // Inner dimension index for A or B
    uint8_t target_buffer = 0; // Target buffer for loads (0 or 1)

    // Constructor for operations without extra operands (like EXECUTE_MAC)
    explicit IROperation(IROpType t) : type(t) {
        // Optionally add checks: assert(t == IROpType::EXECUTE_MAC);
    }

    // Constructor for Load operations
    IROperation(IROpType t, size_t r_or_k, size_t k_or_c, uint8_t buf) : type(t), target_buffer(buf) {
        // Assign indices based on the type
        if (type == IROpType::LOAD_A_ELEMENT) { i = r_or_k; k = k_or_c; }
        else if (type == IROpType::LOAD_B_ELEMENT) { k = r_or_k; j = k_or_c; }
        // Optionally add checks: assert(t == IROpType::LOAD_A_ELEMENT || t == IROpType::LOAD_B_ELEMENT);
    }

     // Constructor for Reset/Store operations
     IROperation(IROpType t, size_t r_idx, size_t c_idx) : type(t), i(r_idx), j(c_idx) {
        // Optionally add checks: assert(t == IROpType::RESET_ACC || t == IROpType::STORE_C_ELEMENT);
     }

};

} // namespace pim 