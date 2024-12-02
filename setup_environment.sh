#!/bin/bash

update_repo () {
   cd $1
   git clean -fd
   git checkout *
   git pull
   cd -
}

if [ ! -d "thirdparty" ]; then
    mkdir thirdparty
fi

if [ ! -d "thirdparty/libcanard" ]; then
    git clone https://github.com/OpenCyphal/libcanard.git thirdparty/libcanard
    cd thirdparty/libcanard
    git checkout 43de1c4966b8d1e5d57978949d63e697f045b358
    git submodule update --init --recursive
    cd -
    patch -d . -p1 < ./patches/transport.py.patch
    patch -d . -p1 < ./patches/canard_stm32.patch
fi

exit 0
