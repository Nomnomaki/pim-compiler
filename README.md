# PIM Compiler

A compiler that converts C++ matrix multiplication code into PIM (Processing-In-Memory) instructions. This compiler specifically handles the multiplication of two matrices (matrix A × matrix B = matrix C).

## Instruction Format

Each PIM instruction is 19 bits wide, structured as follows:
- Bits [18:15]: Opcode (4 bits)
- Bits [14:10]: Operand 1 (5 bits)
- Bits [9:5]: Operand 2 (5 bits)
- Bits [4:0]: Operand 3 (5 bits)

### Supported Operations
- MEM_LOAD (0000): Load data from memory into a register.
- MEM_STORE (0001): Store data from a register to memory.
- MAT_MUL (0010): Perform matrix multiplication using elements from registers.
- ADD (0011): Addition operation (used internally for address calculations, potentially).
- SUB (0100): Subtraction operation (used internally, potentially).

## Compiler Workflow

Here's a step-by-step overview of how the compiler takes input matrices and generates PIM instructions:

1.  **Input Parsing (`main.cpp`)**:
    *   Reads matrix A and matrix B dimensions and elements from standard input.
    *   Stores the matrices in `std::vector<std::vector<int>>`.
    *   Validates that the matrices are compatible for multiplication (inner dimensions match).

2.  **Compilation Request (`main.cpp` -> `compiler.cpp`)**:
    *   Creates a `Compiler` object.
    *   Calls the `compiler.compile(matrix_a, matrix_b)` method.

3.  **Intermediate Representation (IR) Generation (`compiler.cpp`)**:
    *   The `compile` method internally generates a sequence of high-level IR operations representing the matrix multiplication logic. This typically involves nested loops and individual load/multiply/accumulate/store operations.
    *   For example, calculating `C[i][j] += A[i][k] * B[k][j]` is broken down into:
        *   Load `A[i][k]`
        *   Load `B[k][j]`
        *   Multiply loaded values
        *   Accumulate result (potentially using ADD)
        *   Store final `C[i][j]`

4.  **Lookup Table Translation (`compiler.cpp`)**:
    *   Each IR operation is translated into one or more PIM instructions using a lookup mechanism. This is where the specific PIM ISA details are applied.
    *   The compiler determines the correct opcode and calculates the operand values (memory addresses, register numbers) for each PIM instruction based on the IR operation.

5.  **Binary Instruction Generation (`compiler.cpp` -> `instruction.hpp`)**:
    *   Each PIM instruction (opcode and operands) is packed into the 19-bit binary format defined in `instruction.hpp`.
    *   This involves bit shifting and masking to place each part of the instruction into its correct position within the 19 bits.

6.  **Output Generation (`main.cpp`)**:
    *   The `compile` method returns a `std::vector<Instruction>` containing the generated PIM instructions.
    *   `main.cpp` iterates through this vector.
    *   For each `Instruction`, it prints:
        *   A human-readable representation (e.g., `MEM_LOAD R1, A[0][0]`).
        *   The 19-bit binary representation.
        *   The hexadecimal representation of the binary instruction.

## Lookup Table Mechanism

The "lookup table" isn't a literal data structure like a `std::map` in this implementation. Instead, it refers to the **logic within the `Compiler::compile` method** that maps the conceptual steps of matrix multiplication (represented as IR operations) to specific PIM instructions.

Here's how the mapping generally works:

*   **Loading Elements**: To load `A[i][k]`, the compiler:
    *   Calculates the memory address for `A[i][k]` based on the matrix base address and the indices `i`, `k`.
    *   Generates a `MEM_LOAD` instruction (`Opcode::MEM_LOAD`).
    *   Assigns a destination register (e.g., Operand 1).
    *   Places the calculated memory address (or parts of it) into the operand fields (e.g., Operand 2, Operand 3).
*   **Performing Multiplication**: To multiply `A[i][k]` and `B[k][j]` (assuming they are loaded into registers R1 and R2):
    *   Generates a `MAT_MUL` instruction (`Opcode::MAT_MUL`).
    *   Assigns the source registers (R1, R2) and a destination register (R3 for the result) to the operand fields.
*   **Storing Results**: To store the final calculated value of `C[i][j]` from a register (e.g., R4) back to memory:
    *   Calculates the memory address for `C[i][j]`.
    *   Generates a `MEM_STORE` instruction (`Opcode::MEM_STORE`).
    *   Assigns the source register (R4) and the memory address to the operand fields.

The compiler manages register allocation and address calculation as part of this translation process, effectively acting as the "lookup" mechanism that converts the high-level algorithm into low-level PIM instructions.

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

The compiler outputs both the human-readable ISA instructions and their binary/hex representations:
