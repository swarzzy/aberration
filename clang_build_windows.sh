#!/bin/bash
build_type=$1
sh -c "cmake -B_build_clang -H. -G \"CodeBlocks - MinGW Makefiles\" -DCMAKE_BUILD_TYPE=$build_type -DCMAKE_C_COMPILER=C:/msys64/mingw64/bin/clang.exe -DCMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/clang++.exe -DCMAKE_MAKE_PROGRAM=C:/msys64/mingw64/bin/mingw32-make.exe"
sh -c "cmake --build _build_clang --config $build_type"