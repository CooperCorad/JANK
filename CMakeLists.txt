cmake_minimum_required(VERSION 3.2)

project(JANK)

find_package(LLVM 19.1.7 REQUIRED CONFIG) # 14.0.0 origina;
include_directories(include "${LLVM_INCLUDE_DIR}")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(COVERAGE_FLAGS "-g -O0 --coverage -fprofile-arcs -ftest-coverage -fno-exceptions") # -O3 for max optimizations?
SET(CMAKE_CXX_FLAGS_COVERAGE "${COVERAGE_FLAGS}")
SET(CMAKE_C_FLAGS_COVERAGE "${COVERAGE_FLAGS}")

add_subdirectory(src)
add_subdirectory(test)