#!/bin/sh

gcc entity.c \
    3dmath.c \
    gfx.c \
    gui.c \
    boat.c \
    creature.c \
    collision.c \
    light.c \
    model.c \
    physics.c \
    coin.c \
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
    weapon.c \
    main.c \
    -lglfw -lGLU -lGLEW -lGL -lm -O2 \
    -o dungeoner

if [ $? -eq 0 ]
then 
  echo "Successfully compiled." 
  exit 0 
else 
  echo "Failed to compile." >&2 
  exit 1 
fi
