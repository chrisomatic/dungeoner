#!/bin/sh
sh build.sh

if [ $? -eq 0 ]
then 
    ./dungeoner
fi
