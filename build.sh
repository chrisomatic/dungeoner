#!/bin/sh

gcc animation.c \
    3dmath.c \
    camera.c \
    gfx.c \
    gui.c \
    boat.c \
    creature.c \
    collision.c \
    light.c \
    model.c \
    physics.c \
    coin.c \
    consumable.c \
    player.c \
    particles.c \
    projectile.c \
    portal.c \
    shader.c \
    terrain.c \
    text.c \
    timer.c \
    util.c \
    water.c \
    window.c \
    weapon.c \
    socket.c \
    net.c \
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
