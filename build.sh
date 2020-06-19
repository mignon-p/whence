#!/bin/sh

OS=`uname | sed -E -e 's/^(MINGW|CYGWIN|MSYS).*/Windows/'`

case $OS in
    Darwin) exec clang -o whence \
		 -framework CoreFoundation \
		 -lsqlite3 \
                 -liconv \
		 -mmacosx-version-min=10.6 \
		 -Wall -O3 *.c;;
    FreeBSD) exec clang -o whence -Wall -O3 *.c;;
    Linux)   exec gcc   -o whence -Wall -O3 *.c;;
    Windows) exec gcc   -o whence -municode -Wall -O3 *.c;;
    *)       echo \"$OS\" is not a supported OS. && exit 1;;
esac
