#!/bin/sh
cp -p whence   /usr/local/bin             || exit $?
cp -p whence.1 /usr/local/share/man/man1  || exit $?
