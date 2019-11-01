#!/bin/sh

case `uname` in
    Darwin) exec clang -o whence \
		 -framework CoreFoundation \
		 -lsqlite3 \
		 -mmacosx-version-min=10.6 \
		 -Wall -O3 *.c;;
    FreeBSD) exec clang -o whence -Wall -O3 *.c;;
    *) exec gcc -o whence -Wall -O3 *.c;;
esac
