cmake_minimum_required(VERSION 3.10)
project(bsp CXX)

add_subdirectory(third_party/SDL2)
add_subdirectory(third_party/glad)

set(SRC_BSP
	bsp.cpp
	bsp.h
	util_vector.h
)

add_library(bsp STATIC ${SRC_BSP})

set(SRC_EXE
	main.cpp
)

add_executable(bsp_main ${SRC_EXE})
target_link_libraries(bsp_main bsp SDL2 glad)