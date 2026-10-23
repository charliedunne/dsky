# dsky
DSKY Home assistant

dsky can be built for two targets; both are documented below:

| | Native (x86_64) | Raspberry Pi (aarch64) |
|---|---|---|
| How | native build on the x86_64 host | cross-compiled from an x86_64 host (`aarch64-linux-gnu-gcc` + arm64 bookworm sysroot) |
| 3rd-party libraries | `sw/build` | `sw/build-pi` |
| App binary | `sw/build-app/dsky` | `sw/build-app-pi/dsky` |

- [Building (native, x86_64)](#building-native-x86_64)
- [Cross-compiling for Raspberry Pi (aarch64)](#cross-compiling-for-raspberry-pi-aarch64)

## Building (native, x86_64)

### 1. Get the sources

```sh
git clone --recurse-submodules <repo-url> dsky
cd dsky
```

(`--recurse-submodules` fetches SDL, SDL_image, SDL_ttf and pigpio under
`sw/3rdparty`. If you cloned without it, run
`git submodule update --init --recursive` first.)

### 2. Install system dependencies

Install the build tools and system libraries listed in [Dependencies](#dependencies).
On an Arch-based system (e.g. CachyOS):

```sh
sudo pacman -S --needed git cmake make pkgconf \
    libx11 libxext libxcursor libxi libxfixes libxrandr libxrender \
    wayland wayland-protocols libxkbcommon mesa \
    zlib libjpeg-turbo libpng freetype2
```

### 3. Build the third-party libraries

This compiles SDL, SDL2_gfx, SDL_image, SDL_ttf and pigpio from source and
installs them all into `sw/build` (headers in `sw/build/include`, libraries in
`sw/build/lib`). The app build expects exactly this layout:

```sh
cd sw/3rdparty
./build.sh ALL          # or a single lib: SDL, GFX, SDL_image, SDL_ttf, PIGPIO
```

### 4. Build the app

Configure and compile from `sw/` (the CMakeLists links against `./build/lib`,
so the third-party step above must be done first):

```sh
cd sw
cmake -S . -B build-app
cmake --build build-app
```

The binary is produced at `sw/build-app/dsky`.

### 5. Run

The app links against the libraries in `sw/build/lib`, so point the loader at
that directory (or copy the `.so` files into a system library path):

```sh
cd sw
LD_LIBRARY_PATH=build/lib ./build-app/dsky
```

Before first run, create the IPC marker files that `ftok()` uses to derive the
System V message-queue keys:

```sh
sudo touch /var/local/dskykeyint /var/local/dskycmd
```

The app prints a hint if either file is missing.

Notes:

- On x86_64 the window opens normally; on ARM targets CMake switches to
  `SDL_WINDOW_FULLSCREEN` automatically.
- A working display server (X11 or Wayland) and an EGL-capable GPU driver are
  required at runtime (`SDL_RENDERER_ACCELERATED`).
- The pigpio parts only function on Raspberry Pi hardware; on other machines
  the app still builds and runs, GPIO just won't work.

## Cross-compiling for Raspberry Pi (aarch64)

The Pi build is a separate rule from the native one: everything (the five
third-party libraries *and* the app) is compiled with Arch's
`aarch64-linux-gnu-gcc/g++` against a minimal arm64 Debian bookworm sysroot,
so no ARM packages need to be installed on the host.

- Libraries install into `sw/build-pi` (native stays in `sw/build`).
- App binary lands at `sw/build-app-pi/dsky`.

### 1. Host packages (Arch/CachyOS)

```sh
sudo pacman -S --needed debootstrap qemu-user-static qemu-user-static-binfmt \
    aarch64-linux-gnu-gcc
```

`qemu-user-static-binfmt` lets `chroot`/`debootstrap` run arm64 binaries on the
x86_64 host (the sysroot script registers the binfmt rule itself if needed).

### 2. Create the sysroot (one-time, root)

```sh
sudo ./sw/3rdparty/setup-pi-sysroot.sh
```

This debootstraps an arm64 bookworm rootfs into `sw/pi-sysroot` and installs
the SDL/X11/EGL/freetype dev packages inside it. It is idempotent (re-running
is a no-op unless you pass `FORCE=1`) and respects `DSKY_PI_SYSROOT` to place
the rootfs elsewhere.

### 3. Build the third-party libraries for the Pi

```sh
cd sw/3rdparty
./build.sh ALL pi       # "pi" selects cross mode; default is native
```

Every library accepts the mode argument (`SDL`, `GFX`, `SDL_image`, `SDL_ttf`,
`PIGPIO`). Cross mode uses the CMake toolchain file `sw/cmake/pi-aarch64.cmake`
for the CMake-based libs and `--host=aarch64-unknown-linux-gnu` for SDL2_gfx,
and points pkg-config at `sw/build-pi/lib/pkgconfig` so the arm64 `.pc` files
win over any host ones.

### 4. Build the app for the Pi

```sh
cd sw
cmake -S . -B build-app-pi \
    -DCMAKE_TOOLCHAIN_FILE=cmake/pi-aarch64.cmake \
    -DDSKY_DEPS_DIR=$PWD/build-pi
cmake --build build-app-pi
```

`-DDSKY_DEPS_DIR` redirects the include/link directories from `./build` to
`./build-pi`. The toolchain file also makes CMake search for programs on the
host but for libraries/headers only inside the sysroot, so a host SDL2 install
can never leak into the link line.

### 5. Run on the Pi

Copy `sw/build-app-pi/dsky` plus the shared libraries from `sw/build-pi/lib`
(e.g. to `/usr/local/lib`) to the Pi running 64-bit Raspberry Pi OS, then:

```sh
sudo touch /var/local/dskykeyint /var/local/dskycmd   # same as native
LD_LIBRARY_PATH=/usr/local/lib ./dsky
```

On ARM targets CMake sets `SDL_WINDOW_FULLSCREEN`, so the app starts full-screen.
The sysroot is bookworm, which matches Raspberry Pi OS 64-bit (Debian-based);
the runtime only needs the system X11/EGL libraries that ship with the desktop
image.

## Dependencies

### Build tools

- g++ (C++20)
- CMake >= 3.18
- make
- pkg-config
- git (to fetch the submodules under `sw/3rdparty`)

### System libraries

The SDL fork in `sw/3rdparty/SDL` is built from source, so its video-driver
dependencies must be present as development packages. Package names below are
Debian/Ubuntu; Arch-based systems (e.g. CachyOS) use the names in
parentheses:

- X11: `libx11-dev`, `libxext-dev`, `libxcursor-dev`, `libxi-dev`,
  `libxfixes-dev`, `libxrandr-dev`, `libxrender-dev`, `libxss-dev`
  (Arch: `libx11`, `libxext`, `libxcursor`, `libxi`, `libxfixes`,
  `libxrandr`, `libxrender`)
- Wayland: `libwayland-dev` (wayland-client >= 1.18, wayland-egl,
  wayland-cursor), `wayland-protocols`, `libxkbcommon-dev` (>= 0.5.0)
  (Arch: `wayland`, `wayland-protocols`, `libxkbcommon`)
- GPU/EGL: `libegl1-mesa-dev`, `libgles2-mesa-dev`
  (Arch: `mesa`)
  (the app creates an `SDL_RENDERER_ACCELERATED` renderer, so a working
  EGL/GPU driver is required at runtime; `mesa-utils` can be handy for
  checking it)
- Optional: `libdecor-0-dev` (Wayland client-side decorations)

Image formats for the SDL_image fork (JPEG and PNG are enabled by default):

- `zlib1g-dev`, `libjpeg-dev`, `libpng-dev`
  (Arch: `zlib`, `libjpeg-turbo`, `libpng`)
- Optional: `libtiff-dev`, `libwebp-dev`, `libavif-dev`, `libjxl-dev`

Fonts (SDL_ttf, used by `src/hmi/Digits.cpp`):

- `libfreetype-dev` (Arch: `freetype2`) — required to build the SDL_ttf fork
  with `./build.sh SDL_ttf`

Note: pthread and rt come from glibc; System V message queues (`msgget`) are
used for keypad IPC, so no extra packages are needed.

### Libraries built from source (not system packages)

The following live in `sw/3rdparty` as git submodules or a vendored tarball
and are compiled into `sw/build` by `sw/3rdparty/build.sh`:

- SDL (fork, ~2.26.x) — `./build.sh SDL`
- SDL2_gfx 1.0.4 (tarball) — `./build.sh GFX`
- SDL_image (fork, ~2.6.x) — `./build.sh SDL_image`
- pigpio (v79; only functional on Raspberry Pi hardware) — `./build.sh PIGPIO`
- SDL_ttf (needs `libfreetype-dev`) — `./build.sh SDL_ttf`

Build them all with `cd sw/3rdparty && ./build.sh ALL`.

simple-cpp-logger (header-only) is used directly from its include directory;
BSlogger's include path is commented out in CMakeLists.txt and effectively
unused.

### Build workarounds applied by `sw/3rdparty/build.sh`

These are already baked into `build.sh`; they exist because the forks target
older toolchains:

- **CMake >= 4**: SDL and pigpio declare `cmake_minimum_required(VERSION < 3.5)`,
  which CMake 4.x rejects outright. Both builds pass
  `-DCMAKE_POLICY_VERSION_MINIMUM=3.5` to configure anyway.
- **SDL shared library**: CMake defaults to static-only for SDL, so the build
  passes `-DBUILD_SHARED_LIBS=ON` — without it `sw/build/lib` would only get
  `libSDL2.a` and the app's link line (which links `libSDL2.so`) would fail.
- **SDL Pipewire audio**: `-DSDL_PIPEWIRE=OFF`. The system pipewire headers
  (pipewire-0.3) are ABI-incompatible with this SDL fork —
  `pw_node_enum_params()` expects a `struct pw_node *` but the fork passes a
  `struct pw_proxy *`, a hard error under C23. Pipewire audio is simply not
  enabled; ALSA/JACK still work.
- **pigpio under GCC 16 / C23**: pigpio's callback structs use K&R-style empty
  parameter lists (`void (*f)()`), which are no longer compatible with the
  typed function pointers it assigns to them. `pigpio.c`, `pigpiod_if.c` and
  `pigpiod_if2.c` have been patched to store the callbacks as properly typed
  (or `void *`) pointers and cast at each dispatch site; runtime behavior is
  unchanged.

If working with VirtualBox
Display/VMSVGA
Enable 3d Acceleration
64 MB Video memory