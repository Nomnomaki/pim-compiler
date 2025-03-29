# PIM Compiler

A compiler that converts C++ matrix multiplication code into PIM (Processing-In-Memory) instructions. This compiler specifically handles the multiplication of two matrices (matrix A × matrix B = matrix C).

## Instruction Format (19-bit)

## Features

- **Compiler:** Translates matrix multiplication operations into a custom PIM Instruction Set Architecture (ISA).
- **Simulator:** Executes the generated PIM ISA to perform matrix multiplication.
- **Input:** Reads matrix definitions (A and B) from a C++ source file.
- **Output:** Displays the generated PIM instructions and the final resulting matrix C.
- **Testing:** Includes basic unit tests for compiler instruction generation and simulator end-to-end results.

## Building

**Requirements:**

- CMake (version 3.10 or higher)
- A C++ compiler supporting C++17 (e.g., GCC, Clang)

**Build Steps:**

```bash
# 1. Create a build directory (if it doesn't exist)
mkdir -p build

# 2. Navigate into the build directory
cd build

# 3. Configure the project with CMake
cmake ..

# 4. Compile the project
make
```

This will create two executables in the `build` directory:

- `pim_compiler`: The main program for compiling and simulating.
- `pim_tests`: The unit test runner.


## Running the Compiler/Simulator

To run the main program, you need to provide a C++ input file containing the definitions for `matrix_a` and `matrix_b`.

**Command:**

```bash
cd ..
./build/pim_compiler <input_file.cpp>
```

Replace `<input_file.cpp>` with the path to your C++ file containing the matrices.

**Example:**

```bash
./build/pim_compiler ../input_matrices.cpp 
```
(Assuming `input_matrices.cpp` is in the project root, one level above `build`)

**Output:**

The program will:

1. Parse the matrices from the input file.
2. Compile the multiplication into PIM instructions.
3. Print the generated PIM instructions.
4. Initialize the simulator with the matrices.
5. Print the initial state (matrices A, B, C, and compute state).
6. Execute the PIM instructions.
7. Print the final state, including the resulting Matrix C.

## Input File Format

The program uses a very simple parser that expects `matrix_a` and `matrix_b` to be defined in the input C++ file using a specific single-line format.

**Required Format:**

- The entire `std::vector<std::vector<int>> matrix_a = ...;` definition must be on a **single line**.
- The entire `std::vector<std::vector<int>> matrix_b = ...;` definition must be on a **single line**.
- There must be a **space** immediately after the opening `{{` (i.e., `{{ `).
- There must be a **space** immediately before the closing `}};` (i.e., ` }};`).
- Rows are separated by `}, {`.
- Elements within a row are separated by `, `.

**Example (`input_matrices.cpp` content for 2x2 matrices):**

```c++
// Sample input file

#include <vector> // Include is optional for the parser, but good practice

// Matrix A definition
std::vector<std::vector<int>> matrix_a = {{ 1, 2 }, { 3, 4 }};

// Matrix B definition
std::vector<std::vector<int>> matrix_b = {{ 5, 6 }, { 7, 8 }};

// Other C++ code can exist in the file but will be ignored by the parser.
```

**Example (5x5 `matrix_a` format):**

```c++
std::vector<std::vector<int>> matrix_a = {{ 1, 2, 3, 4, 5 }, { 6, 7, 8, 9, 10 }, { 11, 12, 13, 14, 15 }, { 16, 17, 18, 19, 20 }, { 21, 22, 23, 24, 25 }};
```

## Project Structure

```
.
├── CMakeLists.txt
├── input_matrices.cpp  # Example input file
├── include/            # Header files
│   ├── compiler.hpp
│   ├── instruction.hpp
│   └── ir.hpp
├── src/                # Source files
│   ├── main.cpp
│   ├── compiler.cpp
│   └── CMakeLists.txt
└── README.md
```

## Implementation Details

### Instruction Set Architecture (ISA)

The simplified PIM ISA used consists of the following instructions:

1.  `MEM_LOAD Addr=... -> Buffer=...`:
    Loads a value from a memory `Addr`ess into a specific operand `Buffer` (0 or 1).
2.  `MEM_STORE Addr=... <- Accumulator`:
    Stores the current value of the `Accumulator` into a memory `Addr`ess.
3.  `COMPUTE_SETUP`:
    Resets the `Accumulator` to 0.
4.  `COMPUTE_EXEC`:
    Performs a Multiply-Accumulate (MAC) operation: `Accumulator += Buffer[0] * Buffer[1]`.

### Memory Layout (Simulator)

The simulator uses a simple linear memory layout:

- **Matrix A:** Starts at Base Address `0`.
- **Matrix B:** Starts at Base Address `100` (Assumes Matrix A won't exceed 100 elements).
- **Matrix C:** Starts at Base Address `200` (Assumes A and B won't exceed 200 elements combined).

_Note: The simulator's total memory size is calculated dynamically based on the required size for Matrix C._

## Usage

**Note**: This compiler is designed specifically for matrix multiplication operations only. It takes two input matrices (A and B) and produces their product (matrix C).

Input format example:
```cpp
// Matrix A (2x3)
1 2 3
4 5 6

// Matrix B (3x2)
1 2
3 4
5 6
```
