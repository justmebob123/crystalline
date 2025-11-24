#!/bin/bash
cd /workspace/crystalline
make clean
CFLAGS="-Wall -Wextra -g -O0 -fPIC -I./include -mavx2 -mfma" make
cd app
CFLAGS="-Wall -Wextra -g -O0 -std=c11 -I../include -I." make
echo "Build complete with debug symbols"
