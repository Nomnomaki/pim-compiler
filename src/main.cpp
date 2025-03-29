#include "compiler.hpp"
#include "instruction.hpp" // Make sure this includes the new format
// #include "simulator.hpp" // No longer needed
#include <iostream>
#include <iomanip> // Include for std::setw, std::hex, std::setfill, std::left, std::right
#include <vector>
#include <string>
#include <fstream>  // For file input
#include <sstream>  // For parsing lines
#include <stdexcept> // For exceptions

using namespace pim;

// Simple parser for matrix data from a file
// Expects lines like: std::vector<std::vector<int>> matrix_a = {{1, 2}, {3, 4}};
// Returns a pair containing matrix_a and matrix_b
std::pair<std::vector<std::vector<int>>, std::vector<std::vector<int>>> 
parse_matrix_file(const std::string& filename) {
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    std::vector<std::vector<int>> matrix_a, matrix_b;
    std::string line;
    bool found_a = false, found_b = false;

    while (getline(infile, line)) {
        std::string matrix_name; // "matrix_a" or "matrix_b"
        if (line.find("std::vector<std::vector<int>> matrix_a") != std::string::npos) {
            matrix_name = "matrix_a";
        } else if (line.find("std::vector<std::vector<int>> matrix_b") != std::string::npos) {
            matrix_name = "matrix_b";
        } else {
            continue; // Skip lines not defining our matrices
        }

        size_t start_pos = line.find("{{"); // Find start of data {{
        if (start_pos == std::string::npos) {
             std::cerr << "Warning: Could not find matrix start pattern '{{' in line: " << line << std::endl;
             continue;
        }
        size_t end_pos = line.rfind("}}"); // Find end of data }};
        if (end_pos == std::string::npos || end_pos <= start_pos) {
             std::cerr << "Warning: Could not find matrix end pattern '}};' in line: " << line << std::endl;
             continue;
        }

        std::string data_str = line.substr(start_pos + 1, end_pos - start_pos -1); // Get content between {{ and }}

        std::vector<std::vector<int>> current_matrix;
        std::stringstream ss_rows(data_str);
        std::string segment;

        // Check for empty matrix definition like {{}}
         bool isEmptyMatrix = true;
         for(char c : data_str) {
             if (!isspace(c) && c != '{' && c != '}') {
                 isEmptyMatrix = false;
                 break;
             }
         }
         if (isEmptyMatrix && data_str.find('{') == std::string::npos) { // Ensure it's not just whitespace between {}
             current_matrix = {}; // Assign empty matrix
         } else {
            // Existing parsing logic for non-empty matrices
            while(getline(ss_rows, segment, '}')) { // Split by rows "{...}, {..."
                 size_t row_start = segment.find('{');
                 if(row_start == std::string::npos) continue; // Skip segments without '{'

                 std::string row_data = segment.substr(row_start + 1);

                 std::vector<int> row;
                 std::stringstream ss_cols(row_data);
                 std::string val_str;
                 while(getline(ss_cols, val_str, ',')) {
                     // Trim whitespace from val_str
                     size_t first = val_str.find_first_not_of(" \t\n\r");
                     if (std::string::npos == first) continue; // Skip empty segments
                     size_t last = val_str.find_last_not_of(" \t\n\r");
                     val_str = val_str.substr(first, (last - first + 1));

                     if (val_str.empty()) continue; // Skip if becomes empty after trim

                     try {
                         row.push_back(std::stoi(val_str));
                     } catch (const std::invalid_argument& ia) {
                         std::cerr << "Warning: Invalid integer format: '" << val_str << "' in line: " << line << std::endl;
                     } catch (const std::out_of_range& oor) {
                         std::cerr << "Warning: Integer out of range: '" << val_str << "' in line: " << line << std::endl;
                     }
                 }
                 if (!row.empty()) {
                    current_matrix.push_back(row);
                 }
            }
         }

        if (!current_matrix.empty() || isEmptyMatrix) { // Accept empty matrix if parsed as such
             if (matrix_name == "matrix_a") {
                 matrix_a = current_matrix;
                 found_a = true;
             } else if (matrix_name == "matrix_b") {
                 matrix_b = current_matrix;
                 found_b = true;
             }
        }
    }

    if (!found_a || !found_b) {
        throw std::runtime_error("Could not find valid definitions for both matrix_a and matrix_b in file.");
    }

    return {matrix_a, matrix_b};
}

int main(int argc, char* argv[]) {
    if (argc != 2) { // Only expect input file now
        std::cerr << "Usage: " << argv[0] << " <input_cpp_file>\n";
        return 1;
    }
    std::string input_filename = argv[1];

    try {
        // Parse matrices from the input file
        std::cout << "Parsing matrices from " << input_filename << "...\n";
        auto [matrix_a, matrix_b] = parse_matrix_file(input_filename);
        
        // --- Print Parsed Matrices (Optional Debug) ---
        std::cout << "Parsed Matrix A: " << matrix_a.size() << "x" << (matrix_a.empty() ? 0 : matrix_a[0].size()) << "\n";
        std::cout << "Parsed Matrix B: " << matrix_b.size() << "x" << (matrix_b.empty() ? 0 : matrix_b[0].size()) << "\n\n";
        // ---------------------------------------------

        // Create compiler and generate instructions
        std::cout << "Compiling matrix multiplication...\n";
        Compiler compiler;
        std::vector<InstructionWord> instructions = compiler.compile_matrix_mult(matrix_a, matrix_b);
        std::cout << "Generated " << instructions.size() << " instructions (19-bit format packed in uint32_t)\n\n";

        // *** Print generated instructions in a new table format ***
        const int idx_w = 5;
        const int op_txt_w = 15;
        const int core_ptr_w = 8;
        const int rd_w = 3;
        const int wr_w = 3;
        const int row_addr_w = 10;
        const int hex_w = 10; // For the final packed hex output

        // Print Header
        std::cout << std::left
                  << std::setw(idx_w) << "Idx" << " | "
                  << std::setw(op_txt_w) << "Opcode" << " | "
                  << std::right // Right align numeric fields
                  << std::setw(core_ptr_w) << "CorePtr" << " | "
                  << std::setw(rd_w) << "Rd" << " | "
                  << std::setw(wr_w) << "Wr" << " | "
                  << std::setw(row_addr_w) << "Row Addr" << " | "
                  << std::setw(hex_w) << "PackedHex" << std::endl;

        // Print Separator Line
        std::cout << std::string(idx_w, '-') << "-|-"
                  << std::string(op_txt_w, '-') << "-|-"
                  << std::string(core_ptr_w, '-') << "-|-"
                  << std::string(rd_w, '-') << "-|-"
                  << std::string(wr_w, '-') << "-|-"
                  << std::string(row_addr_w, '-') << "-|-"
                  << std::string(hex_w, '-') << std::endl;

        // Print Instruction Rows
        for (size_t i = 0; i < instructions.size(); ++i) {
            InstructionWord packed_instr = instructions[i];
            // Unpack the instruction for display
            instr_format::UnpackedInstr unpacked = instr_format::unpack(packed_instr);

            std::cout << std::left // Left align index and opcode text
                      << std::setw(idx_w) << i << " | "
                      << std::setw(op_txt_w) << instr_format::opcode_to_string(unpacked.opcode) << " | "
                      << std::right << std::setfill(' ') // Right align numbers, space fill
                      << std::setw(core_ptr_w) << unpacked.core_ptr << " | "
                      << std::setw(rd_w) << (unpacked.rd ? "1" : "0") << " | "
                      << std::setw(wr_w) << (unpacked.wr ? "1" : "0") << " | "
                      << std::setw(row_addr_w) << unpacked.row_addr << " | "
                      << std::hex << std::setfill('0') // Hex output, zero fill
                      << std::setw(8) << packed_instr // Print the whole packed word as hex
                      << std::dec << std::setfill(' ') // Reset to decimal and space fill
                      << std::endl;
        }

         // Print Footer Separator Line
        std::cout << std::string(idx_w, '-') << "-|-"
                  << std::string(op_txt_w, '-') << "-|-"
                  << std::string(core_ptr_w, '-') << "-|-"
                  << std::string(rd_w, '-') << "-|-"
                  << std::string(wr_w, '-') << "-|-"
                  << std::string(row_addr_w, '-') << "-|-"
                  << std::string(hex_w, '-') << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
} 