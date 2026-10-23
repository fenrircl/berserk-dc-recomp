@echo off
setlocal
rem Build the Windows port with the Visual Studio environment loaded.
rem Run from anywhere: tools\build-msvc.cmd
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b 1
cmake --build "%~dp0..\build" --target berserk_boot --parallel 4 %*
exit /b %errorlevel%
