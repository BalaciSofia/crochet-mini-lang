cmake_minimum_required(VERSION 3.30)
project(fa_scanner)

set(CMAKE_CXX_STANDARD 20)

add_executable(fa_scanner main.cpp FA.cpp SymbolTable.cpp Scanner.cpp)

# lets main.cpp find tokens.txt, fa/ and examples/ in the repository root
target_compile_definitions(fa_scanner PRIVATE PROJECT_DIR="${CMAKE_SOURCE_DIR}/..")
