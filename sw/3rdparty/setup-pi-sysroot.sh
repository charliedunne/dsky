#!/bin/bash
#
# setup-pi-sysroot.sh - build an arm64 (aarch64) Debian bullseye sysroot for
# cross-compiling dsky to Raspberry Pi OS 64-bit.
#
# The resulting rootfs lives at sw/pi-sysroot (override with DSKY_PI_SYSROOT).
# It provides the arm64 headers/libraries that SDL2 and friends are linked
# against; all compilation still happens on the host with aarch64-linux-gnu-gcc.
#
# Strategy: copy the target Pi's OWN installed headers/libs into the sysroot.
# That guarantees exact version parity with what the app runs against, and it
# sidesteps a mirror problem: bullseye is past its security window, so the
# newest (security) versions in the package lists are no longer fetchable from
# security.debian.org (404). Packages that are NOT installed on the Pi are
# fetched as .debs instead and extracted with plain `ar` + `tar`.
#
# No root, no qemu, no chroot, no debootstrap needed anywhere: only ssh access
# to the Pi is required (default pi@dsky.local, override with DSKY_PI_HOST).
#
# Usage:  ./setup-pi-sysroot.sh           (idempotent - reuses existing sysroot)
#         FORCE=1 ./setup-pi-sysroot.sh   (rebuild from scratch)
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
ROOTFS=${DSKY_PI_SYSROOT:-${SCRIPT_DIR}/../pi-sysroot}
ROOTFS=$(readlink -f "$ROOTFS")
MARKER="${ROOTFS}/.dsky-sysroot-done"
PI_HOST=${DSKY_PI_HOST:-pi@dsky.local}

# Dev packages that must be installed on the Pi (they provide the headers and
# unversioned .so symlinks). The script verifies this and fails with a clear
# message listing anything missing.
REQUIRED_ON_PI=(
    pkg-config
    libx11-dev libxext-dev libxcursor-dev libxi-dev libxfixes-dev
    libxrandr-dev libxrender-dev libxss-dev
    libwayland-dev libxkbcommon-dev
    libegl1-mesa-dev libgles2-mesa-dev
    zlib1g-dev libjpeg62-turbo-dev libpng-dev libfreetype-dev
    libasound2-dev
    # C++ runtime (bullseye = gcc-10): provides the c++/10 headers and, with
    # libc6-dev, the multiarch libstdc++.so.6 that this script turns into a
    # libstdc++.so dev symlink so the cross-gcc links bullseye's SHARED
    # runtime instead of its own static archive (see cmake/pi-aarch64.cmake).
    libstdc++-10-dev
)

# Packages not installed on the Pi: downloaded there with `apt-get download`
# (non-root, straight from the still-served main/rpi repos) and extracted here.
DOWNLOAD_PACKAGES=(wayland-protocols)

command -v ssh >/dev/null || { echo "ERROR: 'ssh' not found" >&2; exit 1; }
command -v ar  >/dev/null || { echo "ERROR: 'ar' not found (install binutils)" >&2; exit 1; }
command -v tar >/dev/null || { echo "ERROR: 'tar' not found" >&2; exit 1; }

if [ -f "$MARKER" ] && [ "${FORCE:-0}" != "1" ]; then
    echo "Sysroot already set up at ${ROOTFS} (remove marker or FORCE=1 to rebuild)"
    exit 0
fi

# --- Step 1: verify the required packages are installed on the Pi -------------
echo "Checking required packages on ${PI_HOST}..."
ssh "$PI_HOST" bash -s -- "${REQUIRED_ON_PI[@]}" <<'EOF'
set -euo pipefail
pkgs=("$@")
missing=()
for p in "${pkgs[@]}"; do
    dpkg-query -W -f'' "$p" >/dev/null 2>&1 || missing+=("$p")
done
if [ ${#missing[@]} -gt 0 ]; then
    echo "ERROR: not installed on the Pi:" >&2
    printf '  %s\n' "${missing[@]}" >&2
    echo "Install them there (e.g. sudo apt-get install <pkg>) and re-run." >&2
    exit 1
fi
echo "OK: all ${#pkgs[@]} required packages are installed on the Pi"
EOF

# --- Step 2: wipe and copy live files from the Pi ------------------------------
if [ -d "$ROOTFS" ]; then
    rm -rf "$ROOTFS"
fi
mkdir -p "$ROOTFS" "$ROOTFS/usr/lib/aarch64-linux-gnu"

echo "Copying headers and GCC runtime from ${PI_HOST}..."
ssh "$PI_HOST" tar -C / -cf - \
    usr/include \
    usr/lib/gcc/aarch64-linux-gnu \
| tar -xf - -C "$ROOTFS"

# Shared objects + pkg-config files: only the top-level .so* entries of the
# multiarch dir (the whole dir is ~1GB on the Pi; we need a small fraction).
echo "Copying shared libraries and pkg-config files from ${PI_HOST}..."
# NOTE: pass the whole pipeline as ONE quoted command. ssh joins argv with
# spaces for the remote shell, so `bash -c 'a && b'` would lose its quoting
# and silently run the wrong thing.
# libc_nonshared.a is a .a (not matched by *.so*) but the cross-gcc spec
# references it unconditionally at final link, so grab it explicitly. The CRT
# objects (crt1.o Scrt1.o crti.o crtn.o) are plain .o files: with --sysroot
# the cross-gcc looks for them in <sysroot>/usr/lib/aarch64-linux-gnu, and
# without them every final link fails with "cannot find crt1.o".
ssh "$PI_HOST" 'cd /usr/lib/aarch64-linux-gnu && find . -maxdepth 1 \( -name "*.so*" -o -name pkgconfig -o -name libc_nonshared.a -o -name "crt*.o" -o -name "Scrt*.o" \) -print0 | tar --null -T - -cf -' \
    | tar -xf - -C "$ROOTFS/usr/lib/aarch64-linux-gnu"

# Make -lstdc++ resolve to bullseye's SHARED runtime. The cross-gcc's own
# LIBRARY_PATH starts with its gcc-cross/10 dir, where libstdc++.so is a
# dangling symlink and libstdc++.a exists: resolved there, -lstdc++ would
# silently STATIC-link the toolchain's own (wrong) libstdc++ into every
# binary. The cmake toolchain file puts <sysroot multiarch> on the command
# line (-L), which ld searches before its built-in dirs; this symlink makes
# that dir win, so binaries link against bullseye's libstdc++.so.6 instead.
# (bullseye itself keeps the dev symlink in the GCC dir, not here - this is a
# cross-sysroot convenience, harmless at runtime: it points at the same .so.6.)
ln -sfn libstdc++.so.6 "$ROOTFS/usr/lib/aarch64-linux-gnu/libstdc++.so"

# Debian keeps libpulsecommon-*.so in a NON-standard subdir (not found by the
# maxdepth-1 find above). libpulse.so.0 has RUNPATH pointing at it, so the Pi
# resolves it at runtime; the cross-linker needs it too to resolve libpulse's
# NEEDED entries.
mkdir -p "$ROOTFS/usr/lib/aarch64-linux-gnu/pulseaudio"
ssh "$PI_HOST" 'cd /usr/lib/aarch64-linux-gnu/pulseaudio && tar -cf - . 2>/dev/null' \
    | tar -xf - -C "$ROOTFS/usr/lib/aarch64-linux-gnu/pulseaudio" || true

# The dynamic linker symlink lives in /usr/lib (not the multiarch dir).
# Recreate it pointing at the real ld we just copied.
LD_LINK=$(ssh "$PI_HOST" readlink /usr/lib/ld-linux-aarch64.so.1)
ln -sfn "$LD_LINK" "$ROOTFS/usr/lib/ld-linux-aarch64.so.1"

# Debian keeps arch-specific generated headers (glibconfig.h, dbus-arch-deps.h,
# ...) in per-package dirs under the multiarch lib dir, referenced by .pc Cflags
# via absolute -I flags. --sysroot only re-roots DEFAULT include dirs, not
# explicit -I paths, and those absolute paths do not exist on the host, so gcc
# would never find them there. Overlay each such dir's CONTENTS into usr/include/
# instead, where the sysroot-re-rooted default search picks them up:
#   <multiarch>/glib-2.0/include/glibconfig.h        -> usr/include/glibconfig.h
#   <multiarch>/dbus-1.0/include/dbus/dbus-arch-deps.h -> usr/include/dbus/dbus-arch-deps.h
echo "Overlaying arch-specific headers into usr/include..."
MULTIARCH_INCLUDES=$(ssh "$PI_HOST" 'ls -d /usr/lib/aarch64-linux-gnu/*/include 2>/dev/null' || true)
for dir in $MULTIARCH_INCLUDES; do
    ssh "$PI_HOST" tar -C "$dir" -cf - . \
        | tar -xf - -C "$ROOTFS/usr/include"
done

# --- Step 3: download + extract the packages missing on the Pi -----------------
DEB_DIR=$(mktemp -d)
trap 'rm -rf "$DEB_DIR"' EXIT
echo "Downloading ${DOWNLOAD_PACKAGES[*]} from ${PI_HOST}..."
ssh "$PI_HOST" bash -s -- "${DOWNLOAD_PACKAGES[@]}" <<'EOF' | tar -xf - -C "$DEB_DIR"
set -euo pipefail
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
cd "$T"
# apt-get writes its progress to stdout; keep the tar stream on stdin clean.
apt-get download "$@" 1>&2
tar -cf - .
EOF

# A .deb is an ar archive: debian-binary, control.tar.*, data.tar.*.
# `ar x` + `tar -xpf data.tar.*` unpacks it as the current user - no chown,
# no root, no qemu. Symlinks and hardlinks are preserved by tar.
for deb in "$DEB_DIR"/*.deb; do
    tmp=$(mktemp -d)
    (
        cd "$tmp"
        ar x "$deb"
        if [ -f data.tar ]; then
            tar -xf data.tar -C "$ROOTFS"
        else
            tar -xpf data.tar.* -C "$ROOTFS"
        fi
    )
    rm -rf "$tmp"
done

# --- Step 4: re-root absolute symlinks ------------------------------------------
# A real bullseye rootfs has /lib -> usr/lib (usr-merge); build.sh and the toolchain
# file probe <sysroot>/lib to detect a valid sysroot. Created first because the pass
# below maps /lib/... targets through it.
ln -sfn usr/lib "$ROOTFS/lib"

# On the Pi, many dev symlinks are ABSOLUTE (e.g. libm.so -> /lib/aarch64-linux-gnu/
# libm.so.6). Copied verbatim, they resolve against the HOST filesystem when opened
# here - i.e. to x86-64 files or missing paths - and ld silently skips them as
# "incompatible", falling through to the cross toolchain prefix's own (wrong) libs.
# Re-point every absolute symlink whose target exists inside the sysroot to a
# RELATIVE one, so it always resolves within $ROOTFS. Symlinks whose mapped target
# is absent (e.g. /etc/alternatives/*) are left as-is; nothing in the build needs them.
echo "Re-rooting absolute symlinks into the sysroot..."
find "$ROOTFS" -type l | while read -r link; do
    tgt=$(readlink "$link")
    case "$tgt" in /*) ;; *) continue ;; esac
    if [ -e "$ROOTFS$tgt" ]; then
        ln -sfr "$ROOTFS$tgt" "$link"
    fi
done

# --- Step 4b: re-root pkg-config .pc files ----------------------------------------
# The .pc files copied from the Pi carry rootfs-relative ABSOLUTE paths: prefix=/usr,
# /usr/include, /usr/lib/aarch64-linux-gnu. Read on the build machine they resolve to
# HOST directories, so pkg-config injects -I/usr/include (host glibc) into compile
# lines, shadowing --sysroot and pulling in host-only symbols (e.g. __isoc23_strtol
# from glibc >= 2.38) that bullseye's libc lacks -> link failure. Re-root every such
# path under $ROOTFS so pkg-config emits sysroot paths instead.
#
# Idempotent: any prior "$ROOTFS/" prefix is stripped before re-applying, so re-running
# (or running against an already-built sysroot) never double-prefixes. Only regular
# files are touched (-type f); .pc symlinks resolve to the one physical file that is
# rewritten in place. GNU sed -i preserves permissions.
echo "Re-rooting pkg-config .pc files into the sysroot..."
ROOTFS_RE=${ROOTFS//./\\.}   # escape dots so $ROOTFS is a literal in the sed pattern
find "$ROOTFS" -name '*.pc' -type f -print0 | while IFS= read -r -d '' pc; do
    sed -E -i \
        -e "s|${ROOTFS_RE}||g" \                            # idempotency: drop a prior "$ROOTFS" prefix (keep the "/usr/...")
        -e "s|/usr/lib|${ROOTFS}/usr/lib|g" \              # /usr/lib and /usr/lib/aarch64-linux-gnu (and subdirs)
        -e "s|/usr/include|${ROOTFS}/usr/include|g" \      # /usr/include (and subdirs)
        -e "s|^prefix=/usr/?|prefix=${ROOTFS}/usr|" \
        -e "s|^exec_prefix=/usr/?|exec_prefix=${ROOTFS}/usr|" \
        -e "s|^original_prefix=/usr/?|original_prefix=${ROOTFS}/usr|" \
        "$pc"
done

# --- Step 5: sanity checks ---------------------------------------------------------

for path in \
    usr/include/stdio.h \
    usr/include/c++/10/new \
    usr/include/aarch64-linux-gnu/c++/10/bits/c++config.h \
    usr/lib/gcc/aarch64-linux-gnu/10/libstdc++.so \
    usr/lib/aarch64-linux-gnu/libc.so \
    usr/lib/aarch64-linux-gnu/libc.so.6 \
    usr/lib/aarch64-linux-gnu/libm.so \
    usr/lib/aarch64-linux-gnu/libm.so.6 \
    usr/lib/aarch64-linux-gnu/libstdc++.so.6 \
    usr/lib/aarch64-linux-gnu/libstdc++.so \
    usr/lib/aarch64-linux-gnu/crt1.o \
    usr/lib/aarch64-linux-gnu/Scrt1.o \
    usr/lib/aarch64-linux-gnu/crti.o \
    usr/lib/aarch64-linux-gnu/crtn.o \
    usr/lib/aarch64-linux-gnu/libX11.so.6 \
    usr/lib/aarch64-linux-gnu/libGLESv2.so \
    usr/lib/aarch64-linux-gnu/libc_nonshared.a \
    usr/lib/aarch64-linux-gnu/pulseaudio/libpulsecommon-14.2.so \
    usr/lib/ld-linux-aarch64.so.1 \
    usr/lib/aarch64-linux-gnu/pkgconfig/x11.pc \
    usr/share/wayland-protocols \
    usr/include/dbus/dbus-arch-deps.h \
    usr/include/glibconfig.h
do
    [ -e "$ROOTFS/$path" ] || { echo "ERROR: sysroot missing $path (incomplete copy?)" >&2; exit 1; }
done

touch "$MARKER"
echo "Done. Sysroot ready at ${ROOTFS}"
