#!/bin/sh

set -xe

CFLAGS="-O3 -std=c99 -Wall -Wextra -pedantic -ggdb -I."
LIBS="-lm"

mkdir -p ./build/

clang $CFLAGS -I./raylib/raylib-5.5_linux_amd64/include/ -o ./build/memeC ./src/memeC.c ./src/menu.c ./src/pairs.c ./src/sequence.c -L./raylib/raylib-5.5_linux_amd64/lib/ -l:libraylib.a $LIBS

x86_64-w64-mingw32-gcc -mwindows -Wall -Wextra -I./raylib/raylib-5.5_win64_mingw-w64/include -o ./build/memeC_windows ./src/memeC.c ./src/menu.c ./src/pairs.c ./src/sequence.c -L./raylib/raylib-5.5_win64_mingw-w64/lib/ -l:libraylib.a -lwinmm -lgdi32 -static
