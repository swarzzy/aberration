#!/bin/bash
build_type=$1
cd ..
sh -c "project/premake/premake5.exe --cc=clang gmake2"
sh -c "mingw32-make config=$1 -j"