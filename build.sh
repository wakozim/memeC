#!/bin/sh

set -xe

CFLAGS="-O3 -std=c99 -Wall -Wextra -pedantic -ggdb -I. `pkg-config --cflags raylib`"
LIBS="-lm -lglfw -ldl -lpthread -lGL -lrt -lX11"

mkdir -p ./build/

clang $CFLAGS -o ./build/memeC ./src/main.c ./raylib/raylib-5.0_linux_amd64/lib/libraylib.a -no-pie -D_DEFAULT_SOURCE $LIBS 
clang $CFLAGS -o ./build/sequence ./src/sequence.c ./raylib/raylib-5.0_linux_amd64/lib/libraylib.a -no-pie -D_DEFAULT_SOURCE $LIBS 
