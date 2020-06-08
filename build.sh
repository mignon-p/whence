#!/bin/bash
if [ `uname` == 'Darwin' ]
then
    exec clang -o whence \
         -framework CoreFoundation \
         -lsqlite3 \
         -mmacosx-version-min=10.6 \
         -Wall -O3 *.c
else
    exec gcc -o whence -Wall -O3 *.c
fi
