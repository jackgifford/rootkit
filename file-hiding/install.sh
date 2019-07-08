#!/bin/sh

make
cp ./file_module  /etc/rc.d/file_module

sudo chmod 0711 /etc/rc.d/file_module
echo "You're good to go 8)" 
