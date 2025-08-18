#!/bin/bash

mkdir -p build
pushd build

COMPILER_FLAGS="-fno-rtti -fno-exceptions -Wall -Werror -Wno-write-strings -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable -DHEX_MAGIC_INTERNAL=1 -DHEX_MAGIC_SLOW=1 -DHEX_MAGIC_LINUX=1"
LINKER_FLAGS="-lSDL2"

g++ $COMPILER_FLAGS ../src/hex_magic.cpp -g -shared -fPIC -o hex_magic_temp.so && mv hex_magic_temp.so hex_magic.so
g++ $COMPILER_FLAGS ../src/linux_hex_magic.cpp -g -o linux_hex_magic $LINKER_FLAGS

popd
