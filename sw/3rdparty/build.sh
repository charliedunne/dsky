#!/bin/bash

ROOT=$(pwd)

# BUILD SDL
cd SDL
cmake -S . -B ./build -DCMAKE_INSTALL_PREFIX:PATH=${ROOT}/../build
cd build
cmake --build  ./
cmake --install .
cd ..
