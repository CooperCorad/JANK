cmake_minimum_required(VERSION 3.31.3)

project(JANK)

set(LLVM_DIR "/Users/coco/cs/llvm-project/build/lib/cmake/llvm")
set(LLVM_SRC_ROOT "/Users/coco/cs/llvm-project/llvm")
set(LLVM_OBJ_ROOT "/Users/coco/cs/llvm-project/llvm")
set(PROJ_SRC_ROOT "/Users/coco/cs/JANK/src")

find_package(LLVM REQUIRED CONFIG) # 14.0.0 original // 19.1.7 release?

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

include_directories(include "${LLVM_INCLUDE_DIR}" "../llvm-project/llvm/include")
message(STATUS "FOUND DIR ${LLVM_INCLUDE_DIR}")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(COVERAGE_FLAGS "-g -O0 -Wall -Wextra -Wpedantic --coverage -fprofile-arcs -ftest-coverage -fno-exceptions") # -O3 for max optimizations?
SET(CMAKE_CXX_FLAGS_COVERAGE "${COVERAGE_FLAGS}")
SET(CMAKE_C_FLAGS_COVERAGE "${COVERAGE_FLAGS}")

add_subdirectory(src)
add_subdirectory(test)
