# CMake toolchain file: cross-compile dsky + 3rdparty libs to aarch64
# (Raspberry Pi OS 64-bit, Debian bullseye based).
#
# Compiler : self-contained GCC 10 cross prefix (default ~/bullseye-cross,
#            override with DSKY_PI_CROSS_PREFIX). It MUST be GCC 10.x to match
#            the Pi's own libstdc++ (GLIBCXX <= 3.4.28): a newer host
#            cross-gcc emits inlined C++ that bullseye's runtime cannot serve,
#            and the app crashes at startup. Recipe: see .diag-notes.md.
# Sysroot  : arm64 Debian bullseye rootfs built by 3rdparty/setup-pi-sysroot.sh
#            (default sw/pi-sysroot, override with DSKY_PI_SYSROOT)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Cross toolchain prefix (self-contained GCC 10, relocatable via $ORIGIN rpath)
if(DEFINED ENV{DSKY_PI_CROSS_PREFIX})
    set(DSKY_PI_CROSS_PREFIX $ENV{DSKY_PI_CROSS_PREFIX})
else()
    set(DSKY_PI_CROSS_PREFIX "$ENV{HOME}/bullseye-cross")
endif()

if(NOT EXISTS "${DSKY_PI_CROSS_PREFIX}/usr/bin/aarch64-linux-gnu-gcc-10")
    message(FATAL_ERROR "Pi cross toolchain not found at '${DSKY_PI_CROSS_PREFIX}' (expected usr/bin/aarch64-linux-gnu-gcc-10). Build it first - see README.md / .diag-notes.md - or point DSKY_PI_CROSS_PREFIX at an existing prefix.")
endif()

set(_DSKY_CROSS_BIN "${DSKY_PI_CROSS_PREFIX}/usr/bin")
set(CMAKE_C_COMPILER   ${_DSKY_CROSS_BIN}/aarch64-linux-gnu-gcc-10)
set(CMAKE_CXX_COMPILER ${_DSKY_CROSS_BIN}/aarch64-linux-gnu-g++-10)
# Pin the binutils to the prefix too, so host tools can never leak in.
set(CMAKE_AR      ${_DSKY_CROSS_BIN}/aarch64-linux-gnu-ar      CACHE FILEPATH "")
set(CMAKE_RANLIB  ${_DSKY_CROSS_BIN}/aarch64-linux-gnu-ranlib  CACHE FILEPATH "")
set(CMAKE_STRIP   ${_DSKY_CROSS_BIN}/aarch64-linux-gnu-strip   CACHE FILEPATH "")
set(CMAKE_OBJCOPY ${_DSKY_CROSS_BIN}/aarch64-linux-gnu-objcopy CACHE FILEPATH "")
set(CMAKE_NM      ${_DSKY_CROSS_BIN}/aarch64-linux-gnu-nm      CACHE FILEPATH "")

# Sysroot location
if(DEFINED ENV{DSKY_PI_SYSROOT})
    set(DSKY_PI_SYSROOT $ENV{DSKY_PI_SYSROOT})
else()
    get_filename_component(DSKY_PI_SYSROOT "${CMAKE_CURRENT_LIST_DIR}/../pi-sysroot" ABSOLUTE)
endif()

if(NOT EXISTS "${DSKY_PI_SYSROOT}/lib")
    message(FATAL_ERROR "Pi sysroot not found at '${DSKY_PI_SYSROOT}'. Create it first: sw/3rdparty/setup-pi-sysroot.sh (see README.md)")
endif()

set(CMAKE_SYSROOT ${DSKY_PI_SYSROOT})

# Because the compiler is GCC 10 - the same major version as the Pi's own gcc -
# its bundled c++/10 headers and libstdc++ match the target exactly: --sysroot
# alone re-roots the glibc includes correctly, and no header isolation
# (-nostdinc++ / -isystem) is needed at all.
#
# The one flag that IS required: -L<sysroot multiarch>. The prefix's built-in
# LIBRARY_PATH lists <prefix>/usr/lib/gcc-cross/aarch64-linux-gnu/10 FIRST, and
# there libstdc++.so is a dangling relative symlink while libstdc++.a exists;
# resolved there, -lstdc++ would silently STATIC-link the wrong (jammy-built)
# libstdc++ into every binary. Command-line -L dirs are searched before
# LIBRARY_PATH, and setup-pi-sysroot.sh puts a `libstdc++.so -> libstdc++.so.6`
# symlink in the sysroot multiarch dir, so bullseye's SHARED runtime wins.

set(_DSKY_MULTIARCH "${DSKY_PI_SYSROOT}/usr/lib/aarch64-linux-gnu")
set(CMAKE_C_FLAGS_INIT "-L${_DSKY_MULTIARCH}")
set(CMAKE_CXX_FLAGS_INIT "-L${_DSKY_MULTIARCH}")

# ld does NOT search -L dirs when resolving NEEDED entries of shared
# libraries (e.g. libstdc++.so -> libm.so.6); point it at the multiarch dir.
# The pulseaudio subdir is needed too: libpulse.so.0 has a RUNPATH there and
# its NEEDED libpulsecommon-*.so lives in that non-standard location.
set(CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,-rpath-link=${_DSKY_MULTIARCH} -Wl,-rpath-link=${_DSKY_MULTIARCH}/pulseaudio")
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
list(APPEND CMAKE_LIBRARY_PATH ${_DSKY_MULTIARCH})
