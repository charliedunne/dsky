#!/bin/bash

ROOT=$(pwd)
ARCH=$(uname -m)

if [ $# -ne 1 ]
then
    echo "Usage: ./build.sh ALL|SDL|GFX"
    exit
fi

# BUILD SDL
function build_sdl() {

    echo "SDL Building..."

    cd SDL
    cmake -S . -B ./build -DCMAKE_INSTALL_PREFIX:PATH=${ROOT}/../build
    cd build
    cmake --build  ./
    cmake --install .
    cd ..
    cd ..
}

# BUILD GFX
function build_gfx() {

    echo "GFX Building..."

    tar -xzvf SDL2_gfx-1.0.4.tar.gz
    cd SDL2_gfx-1.0.4
    if [ ${ARCH} == "aarch64" ]
    then
	# RAPBIAN (aarch64)
	./configure --prefix=${ROOT}/../build --build=aarch64-unknown-linux --disable-mmx
    else
	# Linux (x86_64)
	./configure --prefix=${ROOT}/../build
    fi
    make
    make install
    mkdir -p ${ROOT}/../build/include/GFX
    cp *.h ${ROOT}/../build/include/GFX/
    cd ..
}


if [ $1 == "ALL" ]
then
    build_sdl
    build_gfx

elif [ $1 == "SDL" ]
then
    build_sdl

elif [ $1 == "GFX" ]
then
     build_gfx

fi

exit

