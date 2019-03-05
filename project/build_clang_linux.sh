#!/bin/bash
build_type=$1
cd ..
sh -c "project/premake/premake5 gmake2 --cc=clang"
sh -c "make -j config=$1"