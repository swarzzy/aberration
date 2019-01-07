cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
project(Aberration)

include(../cmake/PrecompiledHeader.cmake)

add_compile_definitions(PRIVATE AB_BUILD_DLL)

set(ABERRATION_SRC_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)

set(SOURCES
	src/ABHeader.cpp
	src/platform/Console.cpp
	)

set(HEADERS
	src/ABHeader.h
	src/Common.h
	src/Types.h
	src/utils/Log.h
	src/platform/Console.h
	src/utils/ABLog.h
	src/utils/Singleton.h
	src/utils/String.h
	src/utils/TypeTraits.h
	Aberration.h
	)

add_library(Aberration SHARED ${HEADERS} ${SOURCES})
add_precompiled_header(Aberration ABHeader.h SOURCE_CXX src/ABHeader.cpp FORCEINCLUDE)
target_include_directories(Aberration PRIVATE ${ABERRATION_SRC_INCLUDE_DIR})
target_link_libraries(Aberration hypermath)

if(WIN32)
	add_compile_definitions(WIN32_LEAN_AND_MEAN)
endif()