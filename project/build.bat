@echo off

set BuildTools=FALSE

set ObjOutDir=build\obj\
set BinOutDir=build\

IF NOT EXIST %BinOutDir% mkdir %BinOutDir%
IF NOT EXIST %ObjOutDir% mkdir %ObjOutDir%

cls

project\ctime -begin build\ab_ctime.ctm

set IncludeDirs=/Iaberration /Ihypermath
set CommonDefines=/DAB_CONFIG_DEBUG /DAB_PLATFORM_WINDOWS /D_CRT_SECURE_NO_WARNINGS 
set LibDefines=/DAB_BUILD_DLL /DWIN32_LEAN_AND_MEAN
set CommonCompilerFlags=/std:c++17 /Gm- /fp:fast /GR- /nologo /diagnostics:classic /WX
set DebugCompilerFlags=/Zi /Od /Ob0 /RTC1 /MTd /Fd%BinOutDir%
set ReleaseCompilerFlags=/Ox /Ob2 /MT /Oi /MT
set LibLinkerFlags=/INCREMENTAL:NO /OPT:REF /MACHINE:X64 /OUT:%BinOutDir%\Aberration.dll /PDB:%BinOutDir%\Aberration.pdb /IMPLIB:%BinOutDir%Aberration.lib user32.lib opengl32.lib gdi32.lib
set AppLinkerFlags=/INCREMENTAL:NO /OPT:REF /MACHINE:X64

set ConfigCompilerFlags=%DebugCompilerFlags%

cl /MP /W3 /Fo%ObjOutDir% %CommonDefines% %LibDefines% %IncludeDirs% %CommonCompilerFlags% %ConfigCompilerFlags% aberration\ab.cpp /LD /link %LibLinkerFlags%

cl /MP /W3 /Fo%ObjOutDir% %CommonDefines% %IncludeDirs% %CommonCompilerFlags% %ConfigCompilerFlags% sandbox\sandbox.cpp /link %AppLinkerFlags% /OUT:%BinOutDir%\Sandbox.exe /PDB:%BinOutDir%\Sandbox.pdb %BinOutDir%\Aberration.lib

IF %BuildTools%==TRUE (
	cl /MP /W3 /Fo%ObjOutDir% %CommonDefines% %IncludeDirs% %CommonCompilerFlags% %ConfigCompilerFlags% tools\AssetBuilder\AssetBuilder.cpp /link %AppLinkerFlags% /OUT:%BinOutDir%\AssetBuilder.exe /PDB:%BinOutDir%\AssetBuilder.pdb
	cl /MP /W3 /Fo%ObjOutDir% %CommonDefines% %IncludeDirs% %CommonCompilerFlags% %ConfigCompilerFlags% tools\FontPreprocessor\FontPreprocessor.cpp /link %AppLinkerFlags% /OUT:%BinOutDir%\FontPreprocessor.exe /PDB:%BinOutDir%\FontPreprocessor.pdb
)

project\ctime -end build\ab_ctime.ctm
