#!/bin/sh

set -xe

CFLAGS="-O3 -std=c99 -Wall -Wextra -pedantic -ggdb -I."
LIBS="-lm -lglfw -ldl -lpthread -lGL -lrt -lX11"

mkdir -p ./build/

clang $CFLAGS -L./raylib/raylib-5.0_linux_amd64/lib/libraylib.a -o ./build/memeC ./src/memeC.c ./src/menu.c ./src/pairs.c ./src/sequence.c -lraylib -no-pie -D_DEFAULT_SOURCE $LIBS 

#x86_64-w64-mingw32-gcc -DPLATFORM_DESKTOP -mwindows -Wall -Wextra -ggdb -I./raylib/raylib-5.0_win64_mingw-w64/include/ $CFLAGS -o ./build/memeC_windows ./src/memeC.c ./src/menu.c ./src/pairs.c ./src/sequence.c -L./raylib/raylib-5.0_win64_mingw-w64/lib -l:libraylib.a -lwinmm -lgdi32 -static

