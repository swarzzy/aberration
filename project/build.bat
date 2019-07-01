
@echo off

set BuildTools=FALSE

set ObjOutDir=build\obj\
set BinOutDir=build\

IF NOT EXIST %BinOutDir% mkdir %BinOutDir%
IF NOT EXIST %ObjOutDir% mkdir %ObjOutDir%

cls

project\ctime -begin build\ab_ctime.ctm

set IncludeDirs=/Ishared /Ihypermath
set CommonDefines=/DAB_CONFIG_DEBUG /DAB_PLATFORM_WINDOWS /D_CRT_SECURE_NO_WARNINGS /DWIN32_LEAN_AND_MEAN
rem set LibDefines=/DAB_BUILD_DLL
set CommonCompilerFlags=/std:c++17 /Gm- /fp:fast /GR- /nologo /diagnostics:classic /WX
rem /RTC1 paste downhere when O0
set DebugCompilerFlags=/Zi /O2 /Ob0 /MTd /Fd%BinOutDir%
set ReleaseCompilerFlags=/Ox /Ob2 /MT /Oi /MT /Zi
set LinkerFlags=/INCREMENTAL:NO /OPT:REF /MACHINE:X64 /OUT:%BinOutDir%\Aberration.exe /PDB:%BinOutDir%\Aberration.pdb user32.lib opengl32.lib gdi32.lib
set AppLinkerFlags=/INCREMENTAL:NO /OPT:REF /MACHINE:X64 /DLL

set ConfigCompilerFlags=%ReleaseCompilerFlags%

set PdbTimestamp=%date:~6,4%-%date:~3,2%-%date:~0,2%-%time:~1,1%-%time:~3,2%-%time:~6,2%

del %BinOutDir%*.pdb >NUL 2>&1

cl /MP /W3 /Fo%ObjOutDir% /DGAME_CODE %CommonDefines% %IncludeDirs% %CommonCompilerFlags% %ConfigCompilerFlags% game\Game.cpp /link %AppLinkerFlags% /OUT:%BinOutDir%\Game.dll /PDB:%BinOutDir%\Game_%PdbTimestamp%.pdb

cl /MP /W3 /Fo%ObjOutDir% /DPLATFORM_CODE %CommonDefines% %IncludeDirs% %CommonCompilerFlags% %ConfigCompilerFlags% platform\windows\Win32Platform.cpp /link %LinkerFlags%

IF %BuildTools%==TRUE (
	cl /MP /W3 /Fo%ObjOutDir% %CommonDefines% %IncludeDirs% %CommonCompilerFlags% %ConfigCompilerFlags% tools\AssetBuilder\AssetBuilder.cpp /link /OUT:%BinOutDir%\AssetBuilder.exe /PDB:%BinOutDir%\AssetBuilder.pdb
	cl /MP /W3 /Fo%ObjOutDir% %CommonDefines% %IncludeDirs% %CommonCompilerFlags% %ConfigCompilerFlags% tools\FontPreprocessor\FontPreprocessor.cpp /link  /OUT:%BinOutDir%\FontPreprocessor.exe /PDB:%BinOutDir%\FontPreprocessor.pdb
)

project\ctime -end build\ab_ctime.ctm
