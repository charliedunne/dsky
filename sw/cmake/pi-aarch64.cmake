# CMake toolchain file: cross-compile dsky + 3rdparty libs to aarch64
# (Raspberry Pi OS 64-bit, Debian bookworm based).
#
# Compiler : Arch's aarch64-linux-gnu-gcc/g++ (pacman -S aarch64-linux-gnu-gcc)
# Sysroot  : arm64 Debian bookworm rootfs built by 3rdparty/setup-pi-sysroot.sh
#            (default sw/pi-sysroot, override with DSKY_PI_SYSROOT)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER   aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
set(CMAKE_AR     aarch64-linux-gnu-ar     CACHE FILEPATH "")
set(CMAKE_RANLIB aarch64-linux-gnu-ranlib CACHE FILEPATH "")

# Sysroot location
if(DEFINED ENV{DSKY_PI_SYSROOT})
    set(DSKY_PI_SYSROOT $ENV{DSKY_PI_SYSROOT})
else()
    get_filename_component(DSKY_PI_SYSROOT "${CMAKE_CURRENT_LIST_DIR}/../pi-sysroot" ABSOLUTE)
endif()
set(CMAKE_SYSROOT ${DSKY_PI_SYSROOT})

# Programs (cmake, ninja, ...) run on the host; libraries/headers come from
# the sysroot. PACKAGE is BOTH so CMake config files installed into our own
# build-pi prefix (outside the sysroot) are still found.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)

# Debian multiarch lib dir inside the sysroot (libX11.so.6 etc. live here)
list(APPEND CMAKE_LIBRARY_PATH ${DSKY_PI_SYSROOT}/usr/lib/aarch64-linux-gnu)
