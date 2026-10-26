#!/usr/bin/env bash
# build-deploy.sh — cross-compile dsky + dskyKeyInt locally, deploy to the Pi,
# and restart the affected systemd services.
#
# Usage: ./deploy/build-deploy.sh [--app] [--daemon] [--resources] [--no-deploy]
#   (no flags = build + deploy both binaries)
#     --app        only the dsky HMI app
#     --daemon     only the dskyKeyInt keypad daemon
#     --resources  also rsync sw/resources/ to /home/pi/run/resources/
#     --no-deploy  build only; do not touch the Pi
#
# After deploying, each service is restarted via systemctl (a warning is
# printed if a unit is not installed yet — run install-services.sh first).
#
# Env overrides: DSKY_PI_HOST (default pi@dsky.local), DSKY_CROSS_CXX
# (default ~/bullseye-cross/usr/bin/aarch64-linux-gnu-g++-10).

set -euo pipefail

ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
SW=$ROOT/sw
DRIVER=$SW/src/drivers/dskyKeyInt
SYSROOT=$SW/pi-sysroot
CROSS_CXX=${DSKY_CROSS_CXX:-$HOME/bullseye-cross/usr/bin/aarch64-linux-gnu-g++-10}
PI_HOST=${DSKY_PI_HOST:-pi@dsky.local}

BUILD_APP=1
BUILD_DAEMON=1
SYNC_RESOURCES=0
DEPLOY=1
for arg in "$@"; do
  case "$arg" in
    --app)       BUILD_DAEMON=0 ;;
    --daemon)    BUILD_APP=0 ;;
    --resources) SYNC_RESOURCES=1 ;;
    --no-deploy) DEPLOY=0 ;;
    *) echo "ERROR: unknown option: $arg (see header)" >&2; exit 1 ;;
  esac
done

ssh_pi() { ssh "$PI_HOST" "$@"; }

# Cross-compile the daemon with the bullseye toolchain.
# Sysroot must be sw/pi-sysroot (GLIBC 2.30); the Pi has GLIBC 2.31.
build_daemon() {
  echo "==> Building dskyKeyInt (cross)..."
  ( cd "$DRIVER" && "$CROSS_CXX" -std=c++17 -D_REENTRANT -O2 \
      -I. -Ibuild \
      -I../../../build-pi/include \
      -I../../../3rdparty/simple-cpp-logger/include \
      -D LOGGER_MAX_LOG_LEVEL_PRINTED=7 -D LOGGER_PREFIX_LEVEL=2 \
      -D LOGGER_ENABLE_COLORS=1 -D LOGGER_ENABLE_COLORS_ON_USER_HEADER=0 \
      -o /tmp/dskyKeyInt_pi main.cpp KeyInt.cpp Pcf8575.cpp MsgQueue.cpp CmdManager.cpp KeypadLed.cpp \
      -L"$SW/build-pi/lib" -lpigpio -lpthread \
      -Wl,-rpath,/home/pi/run \
      -Wl,-rpath-link,"$SYSROOT/usr/lib/aarch64-linux-gnu" \
      --sysroot="$SYSROOT" )
}

# Cross-compile the app with CMake + the pi-aarch64 toolchain file.
build_app() {
  echo "==> Configuring dsky (CMake, pi-aarch64)..."
  ( cd "$SW" && DSKY_PI_SYSROOT=$PWD/pi-sysroot cmake -B build-pi \
      -DCMAKE_TOOLCHAIN_FILE=cmake/pi-aarch64.cmake \
      -DDSKY_DEPS_DIR=$PWD/build-pi \
      -DDSKY_RESOURCE_DIR=/home/pi/run/resources )
  echo "==> Building dsky..."
  ( cd "$SW" && cmake --build build-pi -j"$(nproc)" )
}

# Stage on the Pi, then install + restart. The daemon is root-owned; the app
# stays owned by pi. Restart failures are warnings so a first deploy works
# before the units are installed.
deploy_daemon() {
  echo "==> Deploying dskyKeyInt to ${PI_HOST}:/home/pi/run/..."
  rsync -avz /tmp/dskyKeyInt_pi "$PI_HOST:/tmp/dsky-deploy-dskyKeyInt"
  ssh_pi 'sudo install -m 755 /tmp/dsky-deploy-dskyKeyInt /home/pi/run/dskyKeyInt
rm -f /tmp/dsky-deploy-dskyKeyInt
if systemctl is-active --quiet dskyKeyInt.service; then sudo systemctl restart dskyKeyInt && echo "dskyKeyInt restarted"; else echo "WARN: dskyKeyInt service not active (run install-services.sh)"; fi'
}

deploy_app() {
  echo "==> Deploying dsky to ${PI_HOST}:/home/pi/run/..."
  rsync -avz "$SW/build-pi/dsky" "$PI_HOST:/tmp/dsky-deploy-dsky"
  ssh_pi 'install -m 755 /tmp/dsky-deploy-dsky /home/pi/run/dsky.new
mv -f /home/pi/run/dsky.new /home/pi/run/dsky
rm -f /tmp/dsky-deploy-dsky
if systemctl is-active --quiet dsky.service; then sudo systemctl restart dsky && echo "dsky restarted"; else echo "WARN: dsky service not active (run install-services.sh)"; fi'
}

deploy_resources() {
  echo "==> Syncing resources to ${PI_HOST}:/home/pi/run/resources/..."
  rsync -avz "$SW/resources/" "$PI_HOST:/home/pi/run/resources/"
}

[ "$BUILD_DAEMON" = 1 ] && build_daemon
[ "$BUILD_APP" = 1 ] && build_app

if [ "$DEPLOY" = 1 ]; then
  [ "$SYNC_RESOURCES" = 1 ] && deploy_resources
  [ "$BUILD_DAEMON" = 1 ] && deploy_daemon
  [ "$BUILD_APP" = 1 ] && deploy_app
else
  echo "==> --no-deploy: skipping Pi deployment."
fi

echo "==> Done."
