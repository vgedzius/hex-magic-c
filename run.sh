#!/bin/bash

./build.sh

pushd data

../build/linux_hex_magic

popd
