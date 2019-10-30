cmake_minimum_required(VERSION 3.10)
project(bsp CXX)

add_subdirectory(third_party/SDL2)
add_subdirectory(third_party/glad)

set(SRC_BSP
	bsp.cpp
	bsp.h
	util_vector.h
	poly_part.cpp
	poly_part.h
	linked_list.h

	util_matrix.cpp
	util_matrix.h

	util_intersection.cpp
	util_intersection.h
)

add_library(bsp STATIC ${SRC_BSP})

set(SRC_EXE
	main.cpp

	IInputHandler.h
	IGraphicsEngine.h

	sdl2_core.cpp

	data/shaders/basic.vert.glsl
	data/shaders/basic.frag.glsl
)

add_executable(bsp_main ${SRC_EXE})
target_link_libraries(bsp_main bsp SDL2-static glad)

file(COPY data DESTINATION ${CMAKE_BINARY_DIR})
