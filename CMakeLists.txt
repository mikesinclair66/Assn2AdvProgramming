cmake_minimum_required(VERSION 3.6)
project(Assn2AdvProgramming)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(SOURCE
	${SOURCE}
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Bubble.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ImageOperators.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Position.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Tank.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TankDemo.cpp
)
set(HEADERS
	${HEADERS}
	${CMAKE_CURRENT_SOURCE_DIR}/Attractor.h
	${CMAKE_CURRENT_SOURCE_DIR}/Bubble.h
	${CMAKE_CURRENT_SOURCE_DIR}/defs.h
	${CMAKE_CURRENT_SOURCE_DIR}/GravityAttractor.h
	${CMAKE_CURRENT_SOURCE_DIR}/ImageOperators.h
	${CMAKE_CURRENT_SOURCE_DIR}/Position.h
	${CMAKE_CURRENT_SOURCE_DIR}/SpringAttractor.h
	${CMAKE_CURRENT_SOURCE_DIR}/Tank.h
	${CMAKE_CURRENT_SOURCE_DIR}/TankDemo.h
	${CMAKE_CURRENT_SOURCE_DIR}/WallAttractor.h
)
add_executable(Assn2AdvProgramming ${SOURCE} ${HEADERS})

target_include_directories(Assn2AdvProgramming PUBLIC
                           "${CMAKE_CURRENT_LIST_DIR}/include"
                           )