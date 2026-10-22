#!/bin/bash

ROOT=$(pwd)
ARCH=$(uname -m)

# Build mode: "native" (default) builds for the host into ../build.
# "pi" cross-compiles for aarch64 (Raspberry Pi OS 64-bit) into ../build-pi,
# using cmake/pi-aarch64.cmake and the arm64 sysroot in ../pi-sysroot
# (create it first: sudo ./setup-pi-sysroot.sh).
MODE=${2:-native}

if [ $# -lt 1 ] || [ $1 == "-h" ] || [ $1 == "--help" ]
then
    echo "Usage: ./build.sh ALL|SDL|SDL_ttf|GFX|PIGPIO|SDL_image [native|pi]"
    exit
fi

case "$MODE" in
    native)
        PREFIX=${ROOT}/../build
        BUILDDIR=build
        TOOLCHAIN_ARGS=()
        ;;
    pi)
        PREFIX=${ROOT}/../build-pi
        BUILDDIR=build-pi
        TOOLCHAIN_ARGS=(-DCMAKE_TOOLCHAIN_FILE=${ROOT}/../cmake/pi-aarch64.cmake)
        # Make pkg-config resolve SDL2 (and friends) against our arm64 prefix,
        # not the host's x86_64 system packages.
        export PKG_CONFIG_PATH="${PREFIX}/lib/pkgconfig${PKG_CONFIG_PATH:+:${PKG_CONFIG_PATH}}"
        ;;
    *)
        echo "Unknown build mode '$MODE' (use 'native' or 'pi')"
        exit 1
        ;;
esac

# BUILD SDL
function build_sdl() {

    echo "SDL Building..."

    cd SDL
    rm -rf ${BUILDDIR}
    cmake -S . -B ./${BUILDDIR} -DCMAKE_INSTALL_PREFIX:PATH=${PREFIX} \
        "${TOOLCHAIN_ARGS[@]}" \
        -DBUILD_SHARED_LIBS=ON \
        -DSDL_PIPEWIRE=OFF \
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
    cd ${BUILDDIR}
    cmake --build  ./
    cmake --install .
    cd ..
    cd ..
}

# BUILD SDL_image
function build_sdl_image() {

    echo "SDL_image Building..."

    # In pi mode pin SDL2 to our arm64 prefix so the host's x86_64
    # /lib/cmake/SDL2 is never picked up.
    SDL2_DIR_ARG=()
    if [ "$MODE" == "pi" ]; then
        SDL2_DIR_ARG=(-DSDL2_DIR=${PREFIX}/lib/cmake/SDL2)
    fi

    cd SDL_image
    rm -rf ${BUILDDIR}
    cmake -S . -B ./${BUILDDIR} -DCMAKE_INSTALL_PREFIX:PATH=${PREFIX} \
        "${TOOLCHAIN_ARGS[@]}" \
        "${SDL2_DIR_ARG[@]}"
    cd ${BUILDDIR}
    cmake --build  ./
    cmake --install .
    cd ..
    cd ..
}

# BUILD SDL_ttf
function build_sdl_ttf() {

    echo "SDL_ttf Building..."
    echo "libfreetype-dev must be installed (in the sysroot for pi mode)"

    SDL2_DIR_ARG=()
    if [ "$MODE" == "pi" ]; then
        SDL2_DIR_ARG=(-DSDL2_DIR=${PREFIX}/lib/cmake/SDL2)
    fi

    cd SDL_ttf
    rm -rf ${BUILDDIR}
    cmake -S . -B ./${BUILDDIR} -DCMAKE_INSTALL_PREFIX:PATH=${PREFIX} \
        "${TOOLCHAIN_ARGS[@]}" \
        "${SDL2_DIR_ARG[@]}"
    cd ${BUILDDIR}
    cmake --build  .
    cmake --install .
    cd ..
    cd ..
}

# BUILD GFX
function build_gfx() {

    echo "GFX Building..."

    tar -xzvf SDL2_gfx-1.0.4.tar.gz
    cd SDL2_gfx-1.0.4
    if [ "$MODE" == "pi" ]
    then
	# Cross-compile for aarch64 (Raspberry Pi OS 64-bit)
	CC=aarch64-linux-gnu-gcc CXX=aarch64-linux-gnu-g++ \
	    ./configure --prefix=${PREFIX} --host=aarch64-unknown-linux-gnu --disable-mmx
    elif [ ${ARCH} == "aarch64" ]
    then
	# RAPBIAN (aarch64)
	./configure --prefix=${PREFIX} --build=aarch64-unknown-linux --disable-mmx
    else
	# Linux (x86_64)
	./configure --prefix=${PREFIX}
    fi
    make
    make install
    mkdir -p ${PREFIX}/include/GFX
    cp *.h ${PREFIX}/include/GFX/
    cd ..
}

# BUILD PIGPIO
function build_pigpio() {

    echo "PIGPIO Building..."

    cd pigpio
    # CMake >= 4 dropped support for cmake_minimum_required < 3.5 (pigpio declares 3.0)
    # The callback struct fields in pigpio.c were patched to use the properly-typed
    # function pointers from pigpio.h, so it now compiles cleanly under C23 (GCC >= 16).
    cmake -S . -B ./${BUILDDIR} -DCMAKE_INSTALL_PREFIX:PATH=${PREFIX} \
        "${TOOLCHAIN_ARGS[@]}" \
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
    cd ${BUILDDIR}
    cmake --build  ./
    cmake --install .
    cd ..
    cd ..
}


if [ $1 == "ALL" ]
then
    build_sdl
    build_gfx
    build_sdl_image
    build_sdl_ttf
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
fi

exit
