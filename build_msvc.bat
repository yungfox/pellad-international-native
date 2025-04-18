@echo off
setlocal enabledelayedexpansion

set arch=x64
if "%PROCESSOR_ARCHITECTURE%" == "x86" ( 
    if not defined PROCESSOR_ARCHITEW6432 set arch=x86
)

for /f "tokens=*" %%a in ('"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath') do (set vsinstallpath=%%a)
set vcvarsall=%vsinstallpath%\VC\Auxiliary\Build\vcvarsall.bat

call "%vcvarsall%" %arch% && cl main.c include\glad\glad.c /Fepellad-international /INCLUDE user32.lib opengl32.lib freetype.lib
del /q *.obj
