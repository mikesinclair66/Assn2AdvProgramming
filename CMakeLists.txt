cmake_minimum_required(VERSION 3.6)
project(Assn2AdvProgramming)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCE main.cpp)
add_executable(Assn2AdvProgramming ${SOURCE})

target_include_directories(Assn2AdvProgramming PUBLIC
                           "${CMAKE_CURRENT_LIST_DIR}"
                           )