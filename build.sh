#!/bin/sh

set -xe

CFLAGS="-O3 -std=c99 -Wall -Wextra -pedantic -ggdb -I. `pkg-config --cflags raylib`"
LIBS="-lm -lglfw -ldl -lpthread -lGL -lrt -lX11"

mkdir -p ./build/

clang $CFLAGS -o ./build/memeC ./src/memeC.c ./src/menu.c ./src/pairs.c ./src/sequence.c  ./raylib/raylib-5.0_linux_amd64/lib/libraylib.a -no-pie -D_DEFAULT_SOURCE $LIBS 
