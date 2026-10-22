#!/bin/bash
#
# setup-pi-sysroot.sh - build an arm64 (aarch64) Debian bookworm sysroot for
# cross-compiling dsky to Raspberry Pi OS 64-bit.
#
# The resulting rootfs lives at sw/pi-sysroot (override with DSKY_PI_SYSROOT).
# It provides the arm64 headers/libraries that SDL2 and friends are linked
# against; all compilation still happens on the host with aarch64-linux-gnu-gcc.
#
# Usage:  sudo ./setup-pi-sysroot.sh          (idempotent - reuses existing rootfs)
#         sudo FORCE=1 ./setup-pi-sysroot.sh  (rebuild from scratch)
#
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
ROOTFS=${DSKY_PI_SYSROOT:-${SCRIPT_DIR}/../pi-sysroot}
ROOTFS=$(readlink -f "$ROOTFS")
MIRROR=http://deb.debian.org/debian
MARKER="${ROOTFS}/.dsky-sysroot-done"

# Packages that provide the arm64 headers/libs SDL2 + deps need at link time.
# bookworm matches Raspberry Pi OS 64-bit (both are Debian 12 based).
APT_PACKAGES=(
    pkg-config
    libx11-dev libxext-dev libxcursor-dev libxi-dev libxfixes-dev
    libxrandr-dev libxrender-dev libxss-dev
    libwayland-dev wayland-protocols libxkbcommon-dev
    libegl1-mesa-dev libgles2-mesa-dev
    zlib1g-dev libjpeg62-turbo-dev libpng-dev libfreetype-dev
    libasound2-dev
)

if [ "$(id -u)" -ne 0 ]; then
    echo "ERROR: run as root (sudo ./setup-pi-sysroot.sh)" >&2
    exit 1
fi

command -v debootstrap >/dev/null || { echo "ERROR: debootstrap not installed" >&2; exit 1; }
QEMU_BIN=$(command -v qemu-aarch64-static) || { echo "ERROR: qemu-aarch64-static not installed (pacman -S qemu-user-static)" >&2; exit 1; }

# --- binfmt_misc: required so chroot can execute arm64 ELF (dpkg, apt, ...) ---
if ! mountpoint -q /proc/sys/fs/binfmt_misc; then
    echo "Mounting binfmt_misc..."
    mkdir -p /proc/sys/fs/binfmt_misc
    mount -t binfmt_misc binfmt_misc /proc/sys/fs/binfmt_misc
fi

# Register the aarch64 rule if it is not already active (idempotent).
if [ ! -e /proc/sys/fs/binfmt_misc/qemu-aarch64 ]; then
    echo "Registering qemu-aarch64 binfmt rule..."
    echo ":qemu-aarch64:M::\x7fELF\x02\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\xb7\x00:\xff\xff\xff\xff\xff\xe1\xf8\x00\xff\xff\xff\xff\xff\xff\xff\xff\xfe\xff\xff\xff:${QEMU_BIN}:OCF" \
        > /proc/sys/fs/binfmt_misc/register
fi

# --- debootstrap (first stage) ---------------------------------------------
if [ -f "$MARKER" ] && [ "${FORCE:-0}" != "1" ]; then
    echo "Sysroot already set up at ${ROOTFS} (remove marker or FORCE=1 to rebuild)"
    exit 0
fi

if [ -d "$ROOTFS" ]; then
    rm -rf "$ROOTFS"
fi

echo "Running debootstrap (arm64 minbase) into ${ROOTFS}..."
debootstrap --variant=minbase --foreign arm64 "$ROOTFS" "$MIRROR"

# --- second stage: runs arm64 dpkg via qemu/binfmt --------------------------
echo "Running debootstrap second stage (via qemu-aarch64-static)..."
chroot "$ROOTFS" /debootstrap/debootstrap --second-stage

# apt needs DNS resolution inside the chroot
cp /etc/resolv.conf "${ROOTFS}/etc/resolv.conf"

# --- install arm64 dev packages ---------------------------------------------
echo "Installing arm64 dev packages into sysroot..."
chroot "$ROOTFS" apt-get update
chroot "$ROOTFS" DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends "${APT_PACKAGES[@]}"
chroot "$ROOTFS" apt-get clean
rm -rf "${ROOTFS}/var/lib/apt/lists/"*

touch "$MARKER"
echo "Done. Sysroot ready at ${ROOTFS}"
