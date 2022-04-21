#!/bin/sh
gcc main.c \
    3dmath.c \
    gfx.c \
    creature.c \
    light.c \
    model.c \
    physics.c \
    player.c \
    particles.c \
    projectile.c \
    shader.c \
    terrain.c \
    text.c \
    timer.c \
    util.c \
    water.c \
    window.c \
    -lglfw -lGLU -lGLEW -lGL -lm \
    -o dungeoner

if [ $? -eq 0 ]
then 
  echo "Successfully compiled." 
  exit 0 
else 
  echo "Failed to compile." >&2 
  exit 1 
fi
