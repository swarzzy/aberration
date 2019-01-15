cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
#cmake_policy(SET CMP0076 NEW)
project(Sandbox)

set(SOURCES
	src/Sandbox.cpp
	)

add_executable(Sandbox src/Sandbox.cpp)
target_sources(Sandbox PUBLIC ${SOURCES})
target_link_libraries(Sandbox Aberration)