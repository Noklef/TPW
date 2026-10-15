@echo off
rem Build the hook DLL (build\dinput.dll) and tests (build\tpw_tests.exe), 32-bit.
rem Usage: build.bat [Debug|RelWithDebInfo]   then: build\tpw_tests.exe path\to\tp.exe
rem TPW_SRC, TPW_OUT and TPW_TARGET (environment) build another source tree, into another
rem directory, or one target (tools/fncmp.py builds its mutants so, from a copy).
setlocal
set CONFIG=%1
if "%CONFIG%"=="" set CONFIG=RelWithDebInfo
for /f "usebackq delims=" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set VS=%%i
if not defined VS (echo Visual Studio C++ tools not found & exit /b 1)
call "%VS%\VC\Auxiliary\Build\vcvars32.bat" >nul || exit /b 1
set SRC=%TPW_SRC%
if "%SRC%"=="" set SRC=%~dp0.
set OUT=%TPW_OUT%
if "%OUT%"=="" set OUT=%~dp0build
set TARGET=
if not "%TPW_TARGET%"=="" set TARGET=--target %TPW_TARGET%
cmake -S "%SRC%" -B "%OUT%" -G Ninja -DCMAKE_BUILD_TYPE=%CONFIG% || exit /b 1
cmake --build "%OUT%" %TARGET% || exit /b 1
