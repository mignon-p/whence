#!/bin/bash
if [ `uname` == 'Darwin' ]
then
    exec gcc -o whence -framework CoreFoundation -lsqlite3 -Wall -O3 *.c
else
    exec gcc -o whence -Wall -O3 *.c
fi
