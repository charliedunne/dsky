#!/usr/bin/env bash
# install-services.sh — install the dsky systemd units on the Pi from a dev machine.
#
# Usage: ./deploy/systemd/install-services.sh [--no-start]
#
# Copies dskyKeyInt.service and dsky.service to /etc/systemd/system, runs
# systemctl daemon-reload, and enables + starts both units (omit start with
# --no-start). Safe to re-run after editing a unit file.
#
# Host: pi@dsky.local by default; override with DSKY_PI_HOST.

set -euo pipefail

PI_HOST=${DSKY_PI_HOST:-pi@dsky.local}
UNITS=(dskyKeyInt.service dsky.service)
START=1
[ "${1:-}" = "--no-start" ] && START=0

ssh_pi() { ssh "$PI_HOST" "$@"; }

echo "==> Staging unit files on ${PI_HOST}..."
rsync -avz --mkpath "${UNITS[@]}" "$PI_HOST:/tmp/dsky-units/"

echo "==> Installing units + daemon-reload..."
ssh_pi 'set -e
for f in /tmp/dsky-units/*.service; do
  sudo cp "$f" /etc/systemd/system/
done
sudo systemctl daemon-reload'

if [ "$START" = 1 ]; then
  echo "==> Enabling + starting services..."
  ssh_pi 'sudo systemctl enable --now dskyKeyInt.service dsky.service'
else
  echo "==> Enabling services (not starting; --no-start)..."
  ssh_pi 'sudo systemctl enable dskyKeyInt.service dsky.service'
fi

echo "==> Status:"
ssh_pi 'systemctl --no-pager status dskyKeyInt.service dsky.service || true'
