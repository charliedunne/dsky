#!/bin/bash
# Build wayland-scanner 1.18 natively for the host.
#
# The scanner is a pure text generator (XML in, C code out), so a host build
# is valid for cross-compilation as well. It is used by pi mode of
# ../build.sh to generate Wayland protocol headers that match the Pi's
# libwayland 1.18 ABI: scanners >= 1.20 emit wl_proxy_marshal_flags() calls,
# which do not exist in libwayland-client < 1.20 and fail to link against
# Raspberry Pi OS (bullseye, Wayland 1.18).
#
# Requires only expat development headers (libxml2 is NOT used: the embedded
# DTD validation path is skipped by not defining HAVE_LIBXML).
set -euo pipefail
cd "$(dirname "$0")"

gcc -O2 -Wall -o wayland-scanner scanner.c wayland-util.c -I. -lexpat

./wayland-scanner --version 2>&1 | grep -q "wayland-scanner 1.18.0"
