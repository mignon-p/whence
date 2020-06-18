#!/bin/sh

OS=`uname | sed -e 's/^[Mm][Ii][Nn][Gg][Ww].*/Windows/'`

case $OS in
    Windows) PREFIX="$HOME/AppData/Roaming/local"
             EXE=".exe";;
    *)       PREFIX="/usr/local"
             EXE="";;
esac

install -d -v "$PREFIX/bin" "$PREFIX/share/man/man1"  || exit $?
install -Cpv whence$EXE      "$PREFIX/bin"            || exit $?
install -Cpv -m 644 whence.1 "$PREFIX/share/man/man1" || exit $?
