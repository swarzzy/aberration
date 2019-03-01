#!/bin/bash
build_type=$1
sh -c "tools/premake/premake5 gmake2 --cc=clang"
sh -c "make -j config=$1"