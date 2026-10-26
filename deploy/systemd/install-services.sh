#!/usr/bin/env bash
# install-services.sh — install the dsky systemd units on the Pi from a dev machine.
#
# Usage: ./deploy/systemd/install-services.sh [--no-start]
#
# Copies dskyKeyInt.service and dsky.service to /etc/systemd/system, runs
# systemctl daemon-reload, disables lightdm (the HMI must own /dev/dri/card0;
# an X server would become DRM master and black out the display), and enables
# + starts both units (omit start with --no-start). Safe to re-run after
# editing a unit file.
#
# Host: pi@dsky.local by default; override with DSKY_PI_HOST.

set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
PI_HOST=${DSKY_PI_HOST:-pi@dsky.local}
UNITS=("$SCRIPT_DIR/dskyKeyInt.service" "$SCRIPT_DIR/dsky.service")
START=1
[ "${1:-}" = "--no-start" ] && START=0

ssh_pi() { ssh "$PI_HOST" "$@"; }

echo "==> Staging unit files on ${PI_HOST}..."
# mkdir -p instead of rsync --mkpath: the Pi's rsync (3.2.3) is too old for it.
ssh_pi 'mkdir -p /tmp/dsky-units'
rsync -avz "${UNITS[@]}" "$PI_HOST:/tmp/dsky-units/"

echo "==> Installing units + daemon-reload..."
ssh_pi 'set -e
for f in /tmp/dsky-units/*.service; do
  sudo cp "$f" /etc/systemd/system/
done
sudo systemctl daemon-reload
rm -rf /tmp/dsky-units'

if [ "$START" = 1 ]; then
  echo "==> Disabling lightdm (kiosk: dsky must own /dev/dri/card0)..."
  # Stop X first so the dsky restart below can acquire DRM master cleanly.
  ssh_pi 'sudo systemctl disable --now lightdm'
  echo "==> Enabling + starting services..."
  # enable --now is a no-op for units that are already active, so restart any
  # active unit to make the freshly installed file take effect.
  ssh_pi 'set -e
sudo systemctl enable --now dskyKeyInt.service dsky.service
for u in dskyKeyInt.service dsky.service; do
  if systemctl is-active --quiet "$u"; then sudo systemctl restart "$u"; fi
done'
else
  echo "==> Enabling services (not starting; --no-start)..."
  # Disable lightdm too, but leave it running: --no-start means the caller
  # wants no service state changes beyond enabling.
  ssh_pi 'set -e
sudo systemctl disable lightdm
sudo systemctl enable dskyKeyInt.service dsky.service'
fi

echo "==> Status:"
ssh_pi 'systemctl --no-pager status dskyKeyInt.service dsky.service || true'
