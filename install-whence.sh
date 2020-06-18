#!/bin/sh

OS=`uname | sed -e 's/^[Mm][Ii][Nn][Gg][Ww].*/Windows/'`

case $OS in
    Windows) PREFIX="$HOME/AppData/Roaming/local"
             EXE=".exe";;
    *)       PREFIX="/usr/local"
             EXE="";;
esac

mkdir -p "$PREFIX/bin" "$PREFIX/share/man/man1" || exit $?
cp -p whence$EXE "$PREFIX/bin"                  || exit $?
cp -p whence.1   "$PREFIX/share/man/man1"       || exit $?
