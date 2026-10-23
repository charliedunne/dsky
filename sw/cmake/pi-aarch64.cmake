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

if(NOT EXISTS "${DSKY_PI_SYSROOT}/lib")
    message(FATAL_ERROR "Pi sysroot not found at '${DSKY_PI_SYSROOT}'. Create it first: sudo sw/3rdparty/setup-pi-sysroot.sh (see README.md)")
endif()

set(CMAKE_SYSROOT ${DSKY_PI_SYSROOT})

# Arch's aarch64-linux-gnu-gcc is a vanilla build: with --sysroot it only
# searches <sysroot>/lib, <sysroot>/usr/lib and <sysroot>/usr/include, but
# Debian keeps glibc in the multiarch dir <sysroot>/usr/lib/aarch64-linux-gnu
# (kernel headers in <sysroot>/usr/include/aarch64-linux-gnu). Add both.
#
# Header isolation: the cross-gcc ALSO searches its own prefix
# /usr/aarch64-linux-gnu/include (Arch's host glibc 2.39) after the -I dirs,
# so any top-level header missing from the sysroot would silently mix host and
# sysroot glibc sets. Putting both sysroot include dirs on the -I path fixes
# that: user -I dirs are searched before all defaults.
#
# C++ runtime: the cross-gcc hardcodes its OWN libstdc++.so (built against
# Arch's newer glibc 2.38+), which cannot link against bookworm's glibc 2.36.
# Debian keeps the unversioned libstdc++.so dev symlink in GCC's version dir
# (libstdc++-12-dev installs <sysroot>/usr/lib/gcc/aarch64-linux-gnu/12/).
# User -L flags are searched before gcc's internal dirs, so adding it here
# makes bookworm's libstdc++.so shadow the host one.
# C++ headers: on arm64 Debian, libstdc++-12-dev splits its tree — arch-specific
# overrides (bits/c++config.h & co.) live in
# <sysroot>/usr/include/aarch64-linux-gnu/c++/12, the main tree in
# <sysroot>/usr/include/c++/12. The override dir must come FIRST so
# #include <bits/c++config.h> finds it; and both c++ dirs must precede the
# glibc dirs so #include_next <stdlib.h> (used by <cstdlib> & co.) lands on
# bookworm's glibc instead of the host gcc's C++ wrappers.
set(CMAKE_C_FLAGS_INIT "-I${DSKY_PI_SYSROOT}/usr/include/aarch64-linux-gnu -I${DSKY_PI_SYSROOT}/usr/include -L${DSKY_PI_SYSROOT}/usr/lib/aarch64-linux-gnu -L${DSKY_PI_SYSROOT}/usr/lib/gcc/aarch64-linux-gnu/12")
set(CMAKE_CXX_FLAGS_INIT "-I${DSKY_PI_SYSROOT}/usr/include/aarch64-linux-gnu/c++/12 -I${DSKY_PI_SYSROOT}/usr/include/c++/12 -I${DSKY_PI_SYSROOT}/usr/include/c++/12/backward ${CMAKE_C_FLAGS_INIT}")

# ld does NOT search -L dirs when resolving NEEDED entries of shared
# libraries (e.g. libstdc++.so -> libm.so.6); point it at the multiarch dir.
set(CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,-rpath-link=${DSKY_PI_SYSROOT}/usr/lib/aarch64-linux-gnu")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT}")

# CMake's find_* commands search HOST paths even when CMAKE_SYSROOT is set,
# so packages installed on the build machine (e.g. Arch's /lib64/cmake/SampleRate)
# produce false positives that only blow up later at compile/link time.
#   PROGRAM  NEVER - build-time tools (wayland-scanner, git) run on the host
#   LIBRARY  ONLY  - link against sysroot libs, never host ones
#   INCLUDE  ONLY  - headers from the sysroot only
#   PACKAGE  BOTH  - also search host paths so CMake config files installed
#                    into our own build-pi prefix (outside the sysroot) are
#                    found; SDL's SampleRate false positive is avoided by
#                    passing -DSDL_LIBSAMPLERATE=OFF in 3rdparty/build.sh
set(CMAKE_FIND_ROOT_PATH ${DSKY_PI_SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)

# Debian multiarch lib dir inside the sysroot (libX11.so.6 etc. live here)
list(APPEND CMAKE_LIBRARY_PATH ${DSKY_PI_SYSROOT}/usr/lib/aarch64-linux-gnu)
