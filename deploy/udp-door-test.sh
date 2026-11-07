#!/usr/bin/env bash
# udp-door-test.sh — send a door status signal to the DSKY UDP monitor.
#
# Usage: ./udp-door-test.sh [host] [port] state
#   host   target machine (default: 192.168.2.8, the Pi)
#   port   UDP port (default: 9000)
#   state  1 = positive (DOOR alarm ON), 0 = negative (DOOR alarm OFF);
#          "on"/"off"/"pos"/"neg"/"true"/"false" are accepted and normalized.
#
# Sends a single UDP datagram; the DSKY UdpDoorProgram picks it up within
# ~100 ms and sets/clears the DOOR alarm on the left LCD (solid, not blinking).

set -euo pipefail

HOST="${1:-192.168.2.8}"
PORT="${2:-9000}"
STATE="${3:?usage: $0 [host] [port] 1/0}"

# Normalize the state token; the receiver accepts both forms, but sending a
# canonical 1/0 keeps the journal and this script's output consistent.
case "$STATE" in
  1|on|pos|true)    STATE="1" ;;
  0|off|neg|false)  STATE="0" ;;
  *) echo "error: state must be 1/0 (or on/off/pos/neg)" >&2; exit 1 ;;
esac

if command -v nc >/dev/null 2>&1; then
  printf '%s\n' "$STATE" | nc -u -w1 "$HOST" "$PORT"
else
  # Fallback: bash built-in UDP, no nc required.
  exec 3<>/dev/udp/"$HOST"/"$PORT" || { echo "error: cannot open UDP socket to $HOST:$PORT" >&2; exit 1; }
  printf '%s\n' "$STATE" >&3
  exec 3<&- 3>&-
fi

echo "sent door signal '$STATE' to $HOST:$PORT"
