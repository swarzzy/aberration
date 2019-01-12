cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
project(Aberration)

include(../cmake/PrecompiledHeader.cmake)

add_compile_definitions(PRIVATE AB_BUILD_DLL _CRT_SECURE_NO_WARNINGS)

set(ABERRATION_SRC_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)

set(SOURCES
	src/ABHeader.cpp
	)

set(HEADERS
	src/ABHeader.h
	src/Common.h
	src/Types.h
	src/utils/Log.h
	src/platform/Console.h
	src/platform/Platform.h
	src/utils/ABLog.h
	src/utils/Singleton.h
	src/utils/String.h
	src/utils/TypeTraits.h
	Aberration.h
	)

set(WIN_SOURCES
	src/platform/windows/WinPlatform.cpp
	src/platform/windows/WinConsole.cpp
	)

set(UNIX_SOURCES
	src/platform/unix/UnixPlatform.cpp
	src/platform/unix/UnixConsole.cpp
	)

if (NOT WIN32 AND NOT UNIX)
	error("Unsupported OS")
endif()
if (WIN32)
    set(PLATFORM_SOURCES ${WIN_SOURCES})
endif (WIN32)
if (UNIX)
	set(PLATFORM_SOURCES ${UNIX_SOURCES})
endif()

add_library(Aberration SHARED ${HEADERS} ${SOURCES} ${PLATFORM_SOURCES})
add_precompiled_header(Aberration ABHeader.h SOURCE_CXX src/ABHeader.cpp FORCEINCLUDE)
target_include_directories(Aberration PRIVATE ${ABERRATION_SRC_INCLUDE_DIR})
#target_link_libraries(Aberration hypermath)

if(WIN32)
	add_compile_definitions(WIN32_LEAN_AND_MEAN)
endif()