@echo off
setlocal enabledelayedexpansion

if not exist lib (
    curl.exe -fsSL -o freetype2.zip https://github.com/ubawurinna/freetype-windows-binaries/archive/refs/tags/v2.13.3.zip
    tar.exe -xf freetype2.zip
    if not exist lib mkdir lib
    if not exist lib\freetype2 mkdir lib\freetype2
    if not exist include\freetype2 mkdir include\freetype2
    robocopy freetype-windows-binaries-2.13.3\include\ .\include\freetype2 /e /move > nul
    robocopy "freetype-windows-binaries-2.13.3\release static\vs2015-2022" .\lib\freetype2 /e /move > nul
    del freetype2.zip
    rmdir /s /q freetype-windows-binaries-2.13.3
)
