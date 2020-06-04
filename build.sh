#!/bin/sh
exec gcc -o whence -framework CoreFoundation -Wall -O3 -g *.c
