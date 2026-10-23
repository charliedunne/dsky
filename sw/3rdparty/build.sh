#!/bin/bash
# Fail loudly on any error or unset variable: a silent failure here used to
# let "ALL pi" keep going with missing/broken libraries.
set -euo pipefail

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
        SYSROOT=
        ;;
    pi)
        PREFIX=${ROOT}/../build-pi
        BUILDDIR=build-pi
        TOOLCHAIN_ARGS=(-DCMAKE_TOOLCHAIN_FILE=${ROOT}/../cmake/pi-aarch64.cmake)
        SYSROOT=${DSKY_PI_SYSROOT:-${ROOT}/../pi-sysroot}
        if [ ! -d "${SYSROOT}/lib" ]; then
            echo "ERROR: pi sysroot not found at ${SYSROOT}" >&2
            echo "Create it first:  sudo ./setup-pi-sysroot.sh   (see README.md)" >&2
            exit 1
        fi
        # Isolate pkg-config from the host: PKG_CONFIG_LIBDIR *replaces* the
        # default search dirs, so only our arm64 prefix and the sysroot's own
        # .pc files are visible.
        export PKG_CONFIG_LIBDIR="${PREFIX}/lib/pkgconfig:${SYSROOT}/usr/lib/aarch64-linux-gnu/pkgconfig"
        ;;
    *)
        echo "Unknown build mode '$MODE' (use 'native' or 'pi')"
        exit 1
        ;;
esac

# BUILD SDL
function build_sdl() {

    echo "SDL Building..."

    # In pi mode disable optional features whose CMake checks leak HOST paths
    # and produce false positives that fail later at compile time:
    #  - libsamplerate: find_package(SampleRate) hits Arch's /lib64/cmake/SampleRate
    #    (PACKAGE=BOTH), sets HAVE_LIBSAMPLERATE_H=1, then the build dies on a
    #    samplerate.h that is invisible to the cross compiler.
    #  - libudev: CheckEGL leaves pkg-config cflags (-I/usr/include, as written
    #    in the sysroot's own egl.pc) in CMAKE_REQUIRED_DEFINITIONS, so the
    #    later check_include_file("libudev.h") finds the HOST header; the
    #    sysroot has no libudev-dev.
    SDL_PI_ARGS=()
    if [ "$MODE" == "pi" ]; then
        SDL_PI_ARGS=(-DSDL_LIBSAMPLERATE=OFF -DSDL_LIBUDEV=OFF)
    fi

    cd SDL
    rm -rf ${BUILDDIR}
    cmake -S . -B ./${BUILDDIR} -DCMAKE_INSTALL_PREFIX:PATH=${PREFIX} \
        "${TOOLCHAIN_ARGS[@]}" \
        "${SDL_PI_ARGS[@]}" \
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

    # Re-extract from the clean tarball every time: the tree is untracked and
    # stale in-tree objects (e.g. x86_64 .libs/) would otherwise be reused.
    rm -rf SDL2_gfx-1.0.4
    tar -xzf SDL2_gfx-1.0.4.tar.gz
    cd SDL2_gfx-1.0.4
    if [ "$MODE" == "pi" ]
    then
	# Cross-compile for aarch64 (Raspberry Pi OS 64-bit). --sysroot pins the
	# cross-gcc to our arm64 rootfs instead of Arch's built-in
	# /usr/aarch64-linux-gnu (whose glibc is newer than the Pi's); the
	# CFLAGS/LDFLAGS add Debian's multiarch + base include/lib dirs, which
	# --sysroot alone does not search, keeping host glibc headers out.
	CC="aarch64-linux-gnu-gcc --sysroot=${SYSROOT}" \
	CXX="aarch64-linux-gnu-g++ --sysroot=${SYSROOT}" \
	CFLAGS="-I${SYSROOT}/usr/include/aarch64-linux-gnu -I${SYSROOT}/usr/include" \
	LDFLAGS="-L${SYSROOT}/usr/lib/aarch64-linux-gnu -Wl,-rpath-link=${SYSROOT}/usr/lib/aarch64-linux-gnu" \
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
    # In pi mode hide the host python3: pigpio's install step runs `setup.py install`
    # with it, which would write its bindings into the HOST site-packages. The
    # cross-built libpigpio doesn't need them.
    PIGPIO_PY_ARGS=()
    if [ "$MODE" == "pi" ]; then
        PIGPIO_PY_ARGS=(-DPython_EXECUTABLE=/nonexistent)
    fi
    cmake -S . -B ./${BUILDDIR} -DCMAKE_INSTALL_PREFIX:PATH=${PREFIX} \
        "${TOOLCHAIN_ARGS[@]}" \
        "${PIGPIO_PY_ARGS[@]}" \
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
