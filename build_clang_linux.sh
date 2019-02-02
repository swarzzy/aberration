#!/bin/bash
build_type=$1
sh -c "tools/premake/premake5 --cc=clang gmake2"
sh -c "make config=$1"