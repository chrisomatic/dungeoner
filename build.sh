#!/bin/sh
gcc main.c \
    3dmath.c \
    level.c \
    player.c \
    renderer.c \
    shader.c \
    timer.c \
    window.c \
    -lglfw -lGLU -lGLEW -lGL -lm \
    -o dungeoner
