#!/usr/bin/env bash
# build-deploy.sh — cross-compile dsky + dskyKeyInt locally, deploy to the Pi,
# and restart the affected systemd services. Can also build natively (x86_64)
# for local testing with the stub keypad.
#
# Usage: ./deploy/build-deploy.sh [--app] [--daemon] [--local] [--stub] \
#                                 [--resources] [--no-deploy]
#   (no flags = build + deploy both binaries)
#     --app        only the dsky HMI app
#     --daemon     only the dskyKeyInt keypad daemon
#     --resources  also rsync sw/resources/ to /home/pi/run/resources/
#     --no-deploy  build only; do not touch the Pi
#
# Local (native x86_64) builds:
#   ./deploy/build-deploy.sh --local            app + stub keypad, no deploy
#   ./deploy/build-deploy.sh --local --app      dsky app only
#     --local    build natively with the host compilers (dsky app into
#                sw/build-app, stub keypad into sw/stub/keypad/build);
#                Pi deployment is skipped automatically
#     --stub     (with --local) only the stub keypad
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

# Print usage/help for all supported options and exit.
usage() {
  cat <<'EOF'
Usage: ./deploy/build-deploy.sh [options]

  (no options = build + deploy both binaries to the Pi)

Options:
  --app        only the dsky HMI app
  --daemon     only the dskyKeyInt keypad daemon
  --local      build natively with the host compilers (x86_64): dsky app into
               sw/build-app, stub keypad into sw/stub/keypad/build; Pi
               deployment is skipped automatically
  --stub       (with --local) only the stub keypad
  --resources  also rsync sw/resources/ to /home/pi/run/resources/
  --no-deploy  build only; do not touch the Pi
  -h, --help   show this help and exit

Examples:
  ./deploy/build-deploy.sh                 cross-build + deploy app & daemon
  ./deploy/build-deploy.sh --app           cross-build + deploy app only
  ./deploy/build-deploy.sh --local         native build of app + stub keypad
  ./deploy/build-deploy.sh --local --stub  native build of stub keypad only

Constraints:
  --stub requires --local; --daemon and --resources cannot be combined with
  --local; --app and --stub are mutually exclusive.

Env overrides: DSKY_PI_HOST (default pi@dsky.local), DSKY_CROSS_CXX
(default ~/bullseye-cross/usr/bin/aarch64-linux-gnu-g++-10).
EOF
}

LOCAL_MODE=0
APP_FLAG=""
DAEMON_FLAG=""
STUB_FLAG=""
BUILD_APP=1
BUILD_DAEMON=1
BUILD_STUB=1
SYNC_RESOURCES=0
DEPLOY=1
for arg in "$@"; do
  case "$arg" in
    -h|--help) usage; exit 0 ;;
    --app)       APP_FLAG=1 ;;
    --daemon)    DAEMON_FLAG=1 ;;
    --stub)      STUB_FLAG=1 ;;
    --local)     LOCAL_MODE=1 ;;
    --resources) SYNC_RESOURCES=1 ;;
    --no-deploy) DEPLOY=0 ;;
    *) echo "ERROR: unknown option: $arg (see header)" >&2; exit 1 ;;
  esac
done

# Validate flag combinations
if [ -n "$STUB_FLAG" ] && [ "$LOCAL_MODE" = 0 ]; then
  echo "ERROR: --stub can only be used with --local" >&2
  exit 1
fi
if [ -n "$DAEMON_FLAG" ] && [ "$LOCAL_MODE" = 1 ]; then
  echo "ERROR: --daemon cannot be combined with --local (needs pigpio/Pi)" >&2
  exit 1
fi
if [ "$SYNC_RESOURCES" = 1 ] && [ "$LOCAL_MODE" = 1 ]; then
  echo "ERROR: --resources cannot be combined with --local (no deployment in local mode)" >&2
  exit 1
fi
if [ -n "$APP_FLAG" ] && [ -n "$STUB_FLAG" ]; then
  echo "ERROR: --app and --stub are mutually exclusive (use one or neither)" >&2
  exit 1
fi

# If neither --app nor --daemon given, build both
if [ -n "$APP_FLAG" ]; then BUILD_DAEMON=0; fi
if [ -n "$DAEMON_FLAG" ]; then BUILD_APP=0; fi

# Local mode: native builds only (app + stub keypad), no Pi deployment
if [ "$LOCAL_MODE" = 1 ]; then
  BUILD_DAEMON=0
  DEPLOY=0
  if [ -n "$APP_FLAG" ]; then BUILD_STUB=0; fi
  if [ -n "$STUB_FLAG" ]; then BUILD_APP=0; fi
fi

ssh_pi() { ssh "$PI_HOST" "$@"; }

# Cross-compile the daemon with the bullseye toolchain.
# Sysroot must be sw/pi-sysroot (GLIBC 2.30); the Pi has GLIBC 2.31.
build_daemon() {
  echo "==> Building dskyKeyInt (cross)..."
  ( cd "$DRIVER" && "$CROSS_CXX" -std=c++17 -D_REENTRANT -O2 \
      -I. -Ibuild \
      -I../../core \
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

# Native x86_64 build of the app (host compilers, deps from sw/build).
build_app_local() {
  echo "==> Configuring dsky (CMake, native)..."
  ( cd "$SW" && cmake -B build-app )
  echo "==> Building dsky (native)..."
  ( cd "$SW" && cmake --build build-app -j"$(nproc)" )
}

# Native x86_64 build of the stub keypad (header-only deps, no Pi needed).
build_stub_keypad() {
  echo "==> Configuring stub keypad (CMake, native)..."
  ( cd "$SW" && cmake -S stub/keypad -B stub/keypad/build )
  echo "==> Building stub keypad (native)..."
  ( cd "$SW" && cmake --build stub/keypad/build -j"$(nproc)" )
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

if [ "$LOCAL_MODE" = 1 ]; then
  if [ "$BUILD_APP" = 1 ]; then build_app_local; fi
  if [ "$BUILD_STUB" = 1 ]; then build_stub_keypad; fi
  echo "==> Local mode: skipping Pi deployment."
else
  [ "$BUILD_DAEMON" = 1 ] && build_daemon
  [ "$BUILD_APP" = 1 ] && build_app

  if [ "$DEPLOY" = 1 ]; then
    [ "$SYNC_RESOURCES" = 1 ] && deploy_resources
    [ "$BUILD_DAEMON" = 1 ] && deploy_daemon
    [ "$BUILD_APP" = 1 ] && deploy_app
  else
    echo "==> --no-deploy: skipping Pi deployment."
  fi
fi

echo "==> Done."
