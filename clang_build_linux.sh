#!/bin/bash
build_type=$1
sh -c "cmake -B_build_clang -H. -G \"Unix Makefiles\" -DCMAKE_BUILD_TYPE=$build_type -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++"
sh -c "cmake --build _build_clang --config $build_type"
