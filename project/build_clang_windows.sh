#!/bin/bash

BuildTools=false

ObjOutDir=build/obj/
BinOutDir=build/

mkdir -p $BinOutDir
mkdir -p $ObjOutDir

project/ctime -begin build/ab_ctime.ctm

IncludeDirs="-Iaberration -Ihypermath"
CommonDefines="-DAB_CONFIG_DEBUG -DAB_PLATFORM_WINDOWS -D_CRT_SECURE_NO_WARNINGS" 
LibDefines="-DAB_BUILD_DLL -DWIN32_LEAN_AND_MEAN"
CommonCompilerFlags="-std=c++17 -ffast-math -fno-rtti -fno-exceptions -static-libgcc -static-libstdc++ -fno-strict-aliasing -Werror"
DebugCompilerFlags="-O0 -fno-inline-functions -g"
ReleaseCompilerFlags="-O2 -finline-functions -g"
LibLinkerFlags="-lgdi32 -lopengl32"
AppLinkerFlags="-L$BinOutDir/ -laberration"

ConfigCompilerFlags=$DebugCompilerFlags

clang++ -save-temps=obj -o $BinOutDir/aberration.lib $CommonDefines $LibDefines $IncludeDirs $CommonCompilerFlags $ConfigCompilerFlags -shared aberration/ab.cpp $LibLinkerFlags

clang++ -save-temps=obj -o $BinOutDir/sandbox.exe $CommonDefines $IncludeDirs $CommonCompilerFlags $ConfigCompilerFlags sandbox/sandbox.cpp $AppLinkerFlags

if [[ "$BuildTools" = true ]];
then
clang++ -save-temps=obj -o $BinOutDir/AssetBuilder.exe $CommonDefines $IncludeDirs $CommonCompilerFlags $ConfigCompilerFlags tools/AssetBuilder/AssetBuilder.cpp
clang++ -save-temps=obj -o $BinOutDir/FontPreprocessor.exe $CommonDefines $IncludeDirs $CommonCompilerFlags $ConfigCompilerFlags tools/FontPreprocessor/FontPreprocessor.cpp	
fi
project/ctime -end build/ab_ctime.ctm
