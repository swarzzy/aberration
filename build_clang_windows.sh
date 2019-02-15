#!/bin/bash
build_type=$1
sh -c "tools/premake/premake5.exe --cc=clang gmake2"
sh -c "mingw32-make config=$1 -j"