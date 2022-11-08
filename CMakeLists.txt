cmake_minimum_required(VERSION 3.6)
project(Assn2AdvProgramming)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_executable(Assn2AdvProgramming main.cpp)

target_include_directories(Tutorial PUBLIC
                           "${CMAKE_CURRENT_LIST_DIR}"
                           )