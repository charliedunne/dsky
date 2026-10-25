#!/usr/bin/env bash
# dsky-remote.sh — manage the dsky systemd services on the Pi from a dev machine.
#
# Usage: ./dsky-remote.sh {start|stop|restart|status|tail|enable|disable} [dsky|keyint|all]
#   service defaults to "all" (keyint + dsky).
#     dsky    the HMI app unit        (dsky.service)
#     keyint  the keypad daemon unit  (dskyKeyInt.service)
#     all     both units (default)
#
# All actions are delegated to systemctl on the Pi; logs live in the journal
# (tail = `journalctl -f`). Start order is keyint -> dsky, stop is reversed.
#
# Host: pi@dsky.local by default; override with DSKY_PI_HOST.

set -euo pipefail

PI_HOST=${DSKY_PI_HOST:-pi@dsky.local}

usage() {
  sed -n '2,15p' "$0" | sed 's/^# \{0,1\}//'
  exit "${1:-0}"
}

ssh_pi() { ssh "$PI_HOST" "$@"; }

# Units in dependency order (daemon first). "all" expands to both.
ordered_units() {
  case "${1:-all}" in
    dsky)   echo "dsky.service" ;;
    keyint) echo "dskyKeyInt.service" ;;
    all)    echo "dskyKeyInt.service dsky.service" ;;
    *)      echo "ERROR: unknown service '$1' (use dsky, keyint or all)" >&2; exit 1 ;;
  esac
}

# Reverse order for stopping (app first, then daemon).
reversed_units() {
  local u
  read -r -a units <<< "$(ordered_units "${1:-all}")"
  for (( i=${#units[@]}-1; i>=0; i-- )); do echo "${units[$i]}"; done
}

cmd() {
  local action=$1
  shift
  if [ "$action" = "stop" ] || [ "$action" = "disable" ]; then
    reversed_units "$@" | while read -r u; do
      ssh_pi "sudo systemctl $action $u && echo '$u: $action done'"
    done
  else
    ordered_units "$@" | while read -r u; do
      ssh_pi "sudo systemctl $action $u && echo '$u: $action done'"
    done
  fi
}

case "${1:-}" in
  start)   shift; cmd start "$@" ;;
  stop)    shift; cmd stop "$@" ;;
  restart) shift; cmd restart "$@" ;;
  enable)  shift; cmd enable "$@" ;;
  disable) shift; cmd disable "$@" ;;
  status)
    units=$(ordered_units "${2:-all}")
    ssh_pi "systemctl --no-pager status $units || true"
    ;;
  tail)
    units=$(ordered_units "${2:-all}")
    args=""
    for u in $units; do args="$args -u $u"; done
    ssh_pi "journalctl -f $args"
    ;;
  help|-h|--help) usage 0 ;;
  *)       usage 1 ;;
esac
