#!/bin/bash

VERS=`perl -e 'while (<>) { print $1 if (/#define\s+CMD_VERSION\s+"(.*?)"/); }' whence.h`

exec pod2man \
     --section=1 \
     --center="General Commands Manual" \
     --release="whence $VERS" \
     --stderr \
     whence.pod whence.1
