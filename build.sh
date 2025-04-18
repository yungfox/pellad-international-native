#!/bin/sh

set -xe

PKGS="freetype2"
CFLAGS="-Wall -Wextra"
SRC="main.c include/glad/glad.c"

if [ `uname` = "Darwin" ]; then
    CFLAGS+=" -framework OpenGL -framework Cocoa"
else
    LIBS+="-lX11 -lGL -lXrandr"
fi

clang $CFLAGS `pkg-config --cflags $PKGS` -o pellad-international $SRC `pkg-config --libs $PKGS`
