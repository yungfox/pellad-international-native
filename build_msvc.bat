@echo off
setlocal enabledelayedexpansion

set arch=x64
set freetypelib=win64
if "%PROCESSOR_ARCHITECTURE%" == "x86" ( 
    if not defined PROCESSOR_ARCHITEW6432 set arch=x86
    if not defined PROCESSOR_ARCHITEW6432 set freetypelib=win32
)

if not exist lib (
    curl.exe -fsSL -o freetype2.zip https://github.com/ubawurinna/freetype-windows-binaries/archive/refs/tags/v2.13.3.zip
    tar -xf freetype2.zip
    if not exist lib mkdir lib
    if not exist lib\freetype2 mkdir lib\freetype2
    if not exist include\freetype2 mkdir include\freetype2
    robocopy freetype-windows-binaries-2.13.3\include\ .\include\freetype2 /e /move > nul
    robocopy "freetype-windows-binaries-2.13.3\release static\vs2015-2022" .\lib\freetype2 /e /move > nul
    del freetype2.zip
    rmdir /s /q freetype-windows-binaries-2.13.3
)

for /f "tokens=*" %%a in ('"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath') do (set vsinstallpath=%%a)
set vcvarsall=%vsinstallpath%\VC\Auxiliary\Build\vcvarsall.bat

call "%vcvarsall%" %arch% && cl main.c include\glad\glad.c /Fepellad-international /I include\freetype2 /INCLUDE user32.lib opengl32.lib lib\freetype2\%freetypelib%\freetype.lib /utf-8
del /q *.obj
