#!/bin/bash

exec pod2man \
     --section=1 \
     --center="General Commands Manual" \
     --release="whence 0.9" \
     --stderr \
     whence.pod whence.1
