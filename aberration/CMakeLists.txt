cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
#cmake_policy(SET CMP0076 NEW)
project(Aberration)

set(AB_PLATFORM_UNIX_USE_X11 TRUE)

include(../cmake/PrecompiledHeader.cmake)

# Platform definitions
if(WIN32)
	add_compile_definitions(AB_PLATFORM_WINDOWS)
endif()
if(UNIX)
	add_compile_definitions(AB_PLATFORM_LINUX)
endif()
if(APPLE)
	add_compile_definitions(AB_PLATFORM_MACOS)
endif()

add_compile_definitions(AB_BUILD_DLL _CRT_SECURE_NO_WARNINGS)

set(ABERRATION_SRC_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)

##### LIBRARIES

set(ABERRATION_LIB_WIN32 OpenGL32)
set(ABERRATION_LIB_UNIX X11 GL)

##### SOURCES

set(SOURCES
	src/ABHeader.cpp
	src/utils/String.cpp
	src/platform/API/OpenGL/ABOpenGL.cpp
	)

set(HEADERS
	src/ABHeader.h
	src/Common.h
	src/Types.h
	src/platform/Console.h
	src/platform/Platform.h
	src/platform/Memory.h
	src/platform/Window.h
	src/platform/Input.h
	src/platform/API/OpenGL/ABOpenGL.h
	src/utils/Log.h
	src/utils/String.h
	Aberration.h
	)

set(WIN_SOURCES
	src/platform/windows/Win32Platform.cpp
	src/platform/windows/Win32Console.cpp
	src/platform/windows/Win32Memory.cpp
	src/platform/windows/Win32Window.cpp
	src/platform/windows/Win32WGLContext.h
	src/platform/windows/Win32WGLContext.cpp
	)

set(UNIX_SOURCES
	src/platform/unix/UnixPlatform.cpp
	src/platform/unix/UnixConsole.cpp
	src/platform/unix/UnixMemory.cpp
	)

set(X11_SOURCES
	src/platform/unix/X11Window.cpp
	)

if (NOT WIN32 AND NOT UNIX)
	error("Unsupported OS")
endif()
if (WIN32)
    set(PLATFORM_SOURCES ${WIN_SOURCES})
    set(PLATFORM_LIBS ${ABERRATION_LIB_WIN32})
endif (WIN32)
if (UNIX)
if(AB_PLATFORM_UNIX_USE_X11)
	list(APPEND UNIX_SOURCES ${X11_SOURCES})
endif()
	set(PLATFORM_SOURCES ${UNIX_SOURCES})
	set(PLATFORM_LIBS ${ABERRATION_LIB_UNIX})
endif()

add_library(Aberration SHARED ${HEADERS} ${SOURCES} ${PLATFORM_SOURCES})
target_link_libraries(Aberration ${PLATFORM_LIBS})
add_precompiled_header(Aberration ABHeader.h SOURCE_CXX src/ABHeader.cpp FORCEINCLUDE)
target_include_directories(Aberration PRIVATE ${ABERRATION_SRC_INCLUDE_DIR})
#target_link_libraries(Aberration hypermath)

if(WIN32)
	add_compile_definitions(WIN32_LEAN_AND_MEAN)
endif()