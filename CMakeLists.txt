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
set(MSVC_RELEASE_FLAGS "/Ox /Ob2 /Ot /GL /fp:fast /WX /D \"AB_CONFIG_DISTRIB\"")
set(MSVC_DEBUG_FLAGS "/Od /fp:fast /WX /D \"AB_CONFIG_DEBUG\" /D \"AB_DEBUG_MEMORY\"")
set(MSVC_RELDEB_FLAGS "/Ox /Ob2 /Ot /GL /fp:fast /WX /D \"AB_CONFIG_RELEASE\" /D \"AB_DEBUG_MEMORY\"")

######################################################
## CLANG WINDOWS FLAGS ###############################
if(WIN32)
set(CLANG_RELEASE_FLAGS "-fno-rtti -Werror -O3 -static -D AB_CONFIG_DISTRIB")
set(CLANG_DEBUG_FLAGS "-fno-rtti -Werror -O0 -static -D AB_CONFIG_DEBUG -D AB_DEBUG_MEMORY")
set(CLANG_RELDEB_FLAGS "-fno-rtti -Werror -O3 -static -D AB_CONFIG_RELEASE -D AB_DEBUG_MEMORY")
endif()

######################################################
## CLANG UNIX FLAGS ##################################
if(UNIX)
set(CLANG_RELEASE_FLAGS "-fno-rtti -Werror -O3 -D AB_CONFIG_DISTRIB")
set(CLANG_DEBUG_FLAGS "-fno-rtti -Werror -O0 -D AB_CONFIG_DEBUG -D AB_DEBUG_MEMORY")
set(CLANG_RELDEB_FLAGS "-fno-rtti -Werror -O3 -D AB_CONFIG_RELEASE -D AB_DEBUG_MEMORY")
endif()

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/lib/${CMAKE_BUILD_TYPE}/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/lib/${CMAKE_BUILD_TYPE}/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/bin/${CMAKE_BUILD_TYPE}/)

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
#include_directories(${HYPERMATH_INCLUDE_DIR})
#add_subdirectory(${DEPENDENCIES_DIR}/hypermath)

# Aberration
set(ABERRATION_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/aberration)
include_directories(${ABERRATION_INCLUDE_DIR})
add_subdirectory(aberration)

# Sandbox
add_subdirectory(sandbox)