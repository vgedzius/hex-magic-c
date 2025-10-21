#!/bin/bash

mkdir -p build
pushd build

OPTIND=1
RELEASE=false
RELEASE_FLAGS=""

while getopts "h?r" opt; do
    case "$opt" in
    h|\?)
        echo "Usage: $0 [-r]"
        exit 0
        ;;
    r)  RELEASE=true
        ;;
    esac
done

shift $((OPTIND-1))

[ "${1:-}" = "--" ] && shift

if  $RELEASE;
then
    RELEASE_FLAGS="-O3"
    echo "Building release build..."
else
    echo "Building debug build..."
fi

COMPILER_FLAGS="-fno-rtti -fno-exceptions -Wall -Werror -Wno-write-strings -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable -DHEX_MAGIC_INTERNAL=1 -DHEX_MAGIC_SLOW=1 -DHEX_MAGIC_LINUX=1"
LINKER_FLAGS="-lSDL2"

g++ $COMPILER_FLAGS $RELEASE_FLAGS ../src/hex_magic.cpp -g -shared -fPIC -o hex_magic_temp.so && mv hex_magic_temp.so hex_magic.so
g++ $COMPILER_FLAGS $RELEASE_FLAGS ../src/linux_hex_magic.cpp -g -o linux_hex_magic $LINKER_FLAGS

popd
