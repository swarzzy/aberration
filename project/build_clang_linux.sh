#!/bin/bash

BuildTools=true

#project/ctime -begin _misc/ab_ctime.ctm
ObjOutDir=./build/obj/
BinOutDir=./build/

IncludeDirs="-Iaberration -Ihypermath"
CommonDefines="-DAB_CONFIG_DEBUG -DAB_PLATFORM_LINUX -D_CRT_SECURE_NO_WARNINGS" 
LibDefines="-DAB_BUILD_DLL"
CommonCompilerFlags="-std=c++17 -ffast-math -fno-rtti -fno-exceptions -static-libgcc -static-libstdc++ -fno-strict-aliasing -Werror -march=x86-64 -fPIC -Wl,-rpath=./"
DebugCompilerFlags="-O0 -fno-inline-functions -g"
ReleaseCompilerFlags="-O2 -finline-functions -g"
LibLinkerFlags="-lGL -lX11"
AppLinkerFlags="-L$BinOutDir -laberration"

ConfigCompilerFlags=$DebugCompilerFlags

mkdir -p $BinOutDir
mkdir -p $ObjOutDir

clang++ -save-temps=obj -o $BinOutDir/libaberration.so $CommonDefines $LibDefines $IncludeDirs $CommonCompilerFlags $ConfigCompilerFlags -shared aberration/ab.cpp $LibLinkerFlags

clang++ -save-temps=obj -o $BinOutDir/Sandbox $CommonDefines $IncludeDirs $CommonCompilerFlags $ConfigCompilerFlags sandbox/Sandbox.cpp $AppLinkerFlags

if [[ "$BuildTools" = true ]];
then
clang++ -save-temps=obj -o $BinOutDir/AssetBuilder $CommonDefines $IncludeDirs $CommonCompilerFlags $ConfigCompilerFlags tools/AssetBuilder/AssetBuilder.cpp
clang++ -save-temps=obj -o $BinOutDir/FontPreprocessor $CommonDefines $IncludeDirs $CommonCompilerFlags $ConfigCompilerFlags tools/FontPreprocessor/FontPreprocessor.cpp
fi
#project/ctime -end _misc/ab_ctime.ctm

