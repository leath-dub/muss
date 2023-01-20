#!/bin/sh
# This will build musl libc, its repo should be submodule in lib/
cd ./lib/musl/
./configure
make
