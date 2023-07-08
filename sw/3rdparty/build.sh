#!/bin/bash

ROOT=$(pwd)
ARCH=$(uname -m)

if [ $# -ne 1 ]
then
    echo "Usage: ./build.sh ALL|SDL|SDL_ttf|GFX|PIGPIO|SDL_image|JSON"
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

# BUILD SDL_image
function build_sdl_image() {

    echo "SDL_image Building..."

    cd SDL_image
    cmake -S . -B ./build -DCMAKE_INSTALL_PREFIX:PATH=${ROOT}/../build
    cd build
    cmake --build  ./
    cmake --install .
    cd ..
    cd ..
}

# BUILD SDL_ttf
function build_sdl_ttf() {

    echo "SDL_ttf Building..."
    echo "libfreetype-dev must be installed"

    cd SDL_ttf
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

# BUILD PIGPIO
function build_pigpio() {

    echo "PIGPIO Building..."

    cd pigpio
    cmake -S . -B ./build -DCMAKE_INSTALL_PREFIX:PATH=${ROOT}/../build
    cd build
    cmake --build  ./
    cmake --install .
    cd ..
    cd ..
}

# BUILD JSON
function build_json() {

    echo "JSON Building..."
    cd json
    git submodule init
    git submodule update
    cd ..
    mkdir -p ../build/include/tao
    ln -s ${ROOT}/json/include/tao/json.hpp ../build/include/tao/json.hpp
    ln -s ${ROOT}/json/include/tao/json/ ../build/include/tao/json
    ln -s ${ROOT}/json/external/PEGTL/include/tao/pegtl.hpp ../build/include/tao/pegtl.hpp
    ln -s ${ROOT}/json/external/PEGTL/include/tao/pegtl/ ../build/include/tao/pegtl

}


if [ $1 == "ALL" ]
then
    build_sdl
    build_gfx
    build_sdl_image
    build_pigpio

elif [ $1 == "SDL" ]
then
    build_sdl

elif [ $1 == "GFX" ]
then
     build_gfx

elif [ $1 == "SDL_ttf" ]
then
    build_sdl_ttf

elif [ $1 == "SDL_image" ]
then
    build_sdl_image

elif [ $1 == "PIGPIO" ]
then
    build_pigpio
elif [ $1 == "JSON" ]
then
    build_json
fi

exit

