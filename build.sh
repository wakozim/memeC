#!/bin/sh

set -xe

CFLAGS="-O3 -std=c99 -Wall -Wextra -pedantic -ggdb -I. `pkg-config --cflags raylib`"
LIBS="-lm -lglfw -ldl -lpthread -lGL -lrt -lX11"

mkdir -p ./build/

clang $CFLAGS -o ./build/memeC ./src/memeC.c ./src/menu.c ./src/pairs.c ./src/sequence.c  ./raylib/raylib-5.0_linux_amd64/lib/libraylib.a -no-pie -D_DEFAULT_SOURCE $LIBS 

x86_64-w64-mingw32-gcc -mwindows -Wall -Wextra -ggdb -I./raylib/raylib-5.0_win64_mingw-w64/include/ $CFLAGS -o ./build/memeC_windows ./src/memeC.c ./src/menu.c ./src/pairs.c ./src/sequence.c -L./raylib/raylib-5.0_win64_mingw-w64/lib -l:libraylib.a -lwinmm -lgdi32 -static

