cmake_minimum_required(VERSION 3.10)
project(pim_compiler)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add include directory
include_directories(include)

# Create the main compiler executable
add_executable(pim_compiler
    src/main.cpp
    src/compiler.cpp
) 