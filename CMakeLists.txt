cmake_minimum_required(VERSION 3.6)
project(Assn2AdvProgramming)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
target_include_directories(Tutorial PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )
add_executable(Assn2AdvProgramming main.cpp)