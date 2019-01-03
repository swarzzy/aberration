cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
project(Aberration)

include(../cmake/PrecompiledHeader.cmake)

add_compile_definitions(AB_BUILD_DLL)


set(SOURCES
	src/Common.h
	src/Types.h
	src/utils/Log.h
	src/ABHeader.cpp
	src/ABHeader.h
	)

add_library(Aberration SHARED ${SOURCES})
add_precompiled_header(Aberration ABHeader.h SOURCE_CXX src/ABHeader.cpp FORCEINCLUDE)
target_link_libraries(Aberration hypermath)