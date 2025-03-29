cmake_minimum_required(VERSION 3.10)
project(pim_compiler VERSION 1.0)

# Enable testing
enable_testing()

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add include directory
include_directories(${PROJECT_SOURCE_DIR}/include)

# Add executable
add_executable(pim_compiler
    src/main.cpp
    # Removed src/instruction.cpp
    src/compiler.cpp
    src/simulator.cpp
)

# Add test executable
# We need to link the test executable with the compiler and simulator object files
# or compile them directly for the test executable as well.
# Let's add the necessary source files for the test executable.
add_executable(pim_tests
    src/tests.cpp
    src/compiler.cpp  # Add compiler source
    src/simulator.cpp # Add simulator source
)

# Create the test executable
add_executable(compiler_tests
    src/compiler_tests.cpp
    src/compiler.cpp
)

# Add tests
add_test(NAME BasicMatrixMultiplication COMMAND compiler_tests)

# Optional: Add test labels
set_tests_properties(BasicMatrixMultiplication
    PROPERTIES
    LABELS "compiler"
) 