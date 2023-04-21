#!/bin/bash

ROOT=$(pwd)

# BUILD SDL
#cd SDL
#cmake -S . -B ./build -DCMAKE_INSTALL_PREFIX:PATH=${ROOT}/../build
#cd build
#cmake --build  ./
#cmake --install .
#cd ..
#cd ..

# BUILD GFX
tar -xzvf SDL2_gfx-1.0.4.tar.gz
cd SDL2_gfx-1.0.4
./configure --prefix=${ROOT}/../build
make
make install
mkdir -p ${ROOT}/../build/include/GFX
cp *.h ${ROOT}/../build/include/GFX/
cd..

