cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
project(Aberration)

add_compile_definitions(AB_BUILD_DLL)

set(SOURCES
	src/ab.cpp
	)

add_library(Aberration SHARED ${SOURCES})
target_link_libraries(Aberration hypermath)