cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
set(CMAKE_CONFIGURATION_TYPES Debug Release RelWithDebInfo CACHE STRING"" INTERNAL FORCE)
project(Aberration-engine)
set(CMAKE_CXX_STANDARD 17)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

######################################################
## VARIABLES #########################################
set(DEPENDENCIES_DIR ${CMAKE_SOURCE_DIR}/dependencies)

######################################################
## MSVC FLAGS ########################################
set(MSVC_RELEASE_FLAGS "/Ox /Ob2 /Ot /GL /fp:fast")
set(MSVC_DEBUG_FLAGS "/Od /fp:fast")
set(MSVC_RELDEB_FLAGS "/Ox /Ob2 /Ot /GL /fp:fast")

######################################################
## CLANG FLAGS #######################################
set(CLANG_RELEASE_FLAGS "-O3")
set(CLANG_DEBUG_FLAGS "-O0")
set(CLANG_RELDEB_FLAGS "-O3")

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/bin)

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

# Compiler flags
if(MSVC)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${MSVC_RELEASE_FLAGS}")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${MSVC_DEBUG_FLAGS}")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${MSVC_RELDEB_FLAGS}")
else()
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${CLANG_RELEASE_FLAGS}")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CLANG_DEBUG_FLAGS}")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${CLANG_RELDEB_FLAGS}")
endif()

# Hypermath
set(HYPERMATH_INCLUDE_DIR ${DEPENDENCIES_DIR}/hypermath)
add_subdirectory(${DEPENDENCIES_DIR}/hypermath)
include_directories(${HYPERMATH_INCLUDE_DIR})

# Aberration
set(ABERRATION_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/Aberration)
add_subdirectory(Aberration)
include_directories(${ABERRATION_INCLUDE_DIR})

# Sandbox
add_subdirectory(Sandbox)