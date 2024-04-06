#!/bin/sh

set -xe

CFLAGS="-O3 -std=c99 -Wall -Wextra -pedantic -ggdb -I. `pkg-config --cflags raylib`"
LIBS="-lm `pkg-config --libs raylib` -lglfw -ldl -lpthread"

mkdir -p ./build/

clang $CFLAGS -o ./build/memeC ./src/main.c $LIBS
clang $CFLAGS -o ./build/sequence ./src/sequence.c $LIBS
