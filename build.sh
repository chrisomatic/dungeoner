#!/bin/sh
gcc main.c \
    player.c \
    renderer.c \
    shader.c \
    timer.c \
    window.c \
    -lglfw -lGLU -lGLEW -lGL -lm \
    -o dungeoner
