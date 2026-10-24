#!/usr/bin/env bash
# dsky-remote.sh — start/stop/status the dsky app on the Pi from a dev machine.
#
# Usage: ./dsky-remote.sh {start|stop|status|restart|tail}
#
# The app must already be deployed to ~/run on the Pi (binary + lib/*.so* +
# resources/ — see .diag-notes.md, "DEPLOYMENT + ON-PI VERIFICATION").
#
# Host: pi@dsky.local by default; override with DSKY_PI_HOST.
# Run dir: ~/run by default; override with DSKY_RUN_DIR (relative to $HOME).
#
# Operational notes baked in from .diag-notes.md:
# - Launch is detached with setsid inside the desktop session so it survives
#   ssh disconnect, and needs DISPLAY/XAUTHORITY/LD_LIBRARY_PATH set.
# - The remote command uses ';' (not '&&') before the backgrounded launch:
#   with an AND-list, `A && B &` backgrounds the *whole list* and a subshell
#   holding ssh's stdout pipe stays alive until dsky exits → ssh hangs.
# - Stop sends SIGINT: it is the only signal main.cpp handles, and only a
#   clean exit flushes the block-buffered stdio log. SIGTERM/SIGKILL would
#   leave the log unflushed. SIGKILL is used only as a last resort.
# - The log (dsky.log) is block-buffered: it stays empty while the app runs
#   idle and fills on buffer-full or clean exit. Each start archives the
#   previous session to dsky.log.old.

set -euo pipefail

PI_HOST=${DSKY_PI_HOST:-pi@dsky.local}
RUN_DIR=${DSKY_RUN_DIR:-run}
LOG_FILE=dsky.log

usage() {
  sed -n '2,12p' "$0" | sed 's/^# \{0,1\}//'
  exit "${1:-0}"
}

ssh_pi() { ssh "$PI_HOST" "$@"; }

# PIDs of running dsky instances on the Pi (empty string when none).
pids_on_pi() { ssh_pi "pgrep -x dsky || true"; }

cmd_start() {
  local pids
  pids=$(pids_on_pi)
  if [ -n "$pids" ]; then
    echo "ERROR: dsky already running on ${PI_HOST} (pid $(echo $pids | tr '\n' ' ')). Use 'restart'." >&2
    return 1
  fi

  # Archive the previous session's log, then launch detached. The trailing
  # `echo started` lets ssh return immediately; pgrep below is authoritative.
  ssh_pi "cd \$HOME/${RUN_DIR}; [ -f ${LOG_FILE} ] && mv -f ${LOG_FILE} ${LOG_FILE}.old; setsid env DISPLAY=:0 XAUTHORITY=\$HOME/.Xauthority LD_LIBRARY_PATH=\$HOME/${RUN_DIR} ./dsky > ${LOG_FILE} 2>&1 < /dev/null & echo started"

  sleep 1
  pids=$(pids_on_pi)
  if [ -z "$pids" ]; then
    echo "ERROR: dsky did not stay up. Last log lines:" >&2
    ssh_pi "tail -n 20 \$HOME/${RUN_DIR}/${LOG_FILE}" || true
    return 1
  fi
  echo "dsky started on ${PI_HOST} (pid $(echo $pids | tr '\n' ' '))."
}

cmd_stop() {
  # Single remote round-trip: SIGINT, wait up to 10 s for clean exit,
  # escalate to SIGKILL only if the app is wedged. Prints the outcome.
  local result
  result=$(ssh_pi 'pids=$(pgrep -x dsky || true); [ -z "$pids" ] && { echo NOT_RUNNING; exit 0; }; kill -INT $pids; for i in $(seq 1 40); do pgrep -x dsky >/dev/null 2>&1 || break; sleep 0.25; done; if pgrep -x dsky >/dev/null 2>&1; then kill -KILL $pids; echo KILLED; else echo STOPPED; fi')

  case "$result" in
    NOT_RUNNING) echo "dsky is not running on ${PI_HOST}." ;;
    STOPPED)
      echo "dsky stopped cleanly. Last log lines:"
      ssh_pi "tail -n 5 \$HOME/${RUN_DIR}/${LOG_FILE}" || true
      ;;
    KILLED)
      echo "WARN: dsky ignored SIGINT for 10 s; sent SIGKILL (log may be unflushed)." >&2
      ;;
    *)
      echo "ERROR: unexpected stop result: ${result}" >&2
      return 1
      ;;
  esac
}

cmd_status() {
  local pids
  pids=$(pids_on_pi)
  if [ -z "$pids" ]; then
    echo "dsky is not running on ${PI_HOST}."
    return 1
  fi
  # One remote round-trip: per-pid uptime + RSS, then the log tail.
  ssh_pi bash -s -- "$RUN_DIR" "$LOG_FILE" <<'EOF'
run_dir=$1; log_file=$2
for p in $(pgrep -x dsky); do
  et=$(ps -o etime= -p $p | tr -d ' ')
  rss=$(awk '/VmRSS/{print $2" "$3}' /proc/$p/status)
  echo "pid=$p uptime=$et rss=${rss:-?}"
done
echo "--- ${log_file} (tail) ---"
if [ -s "$HOME/${run_dir}/${log_file}" ]; then
  tail -n 10 "$HOME/${run_dir}/${log_file}"
else
  echo "(empty — block-buffered stdio; fills on buffer-full or clean exit)"
fi
EOF
}

cmd_tail() {
  ssh_pi "tail -n +1 -f \$HOME/${RUN_DIR}/${LOG_FILE}"
}

case "${1:-}" in
  start)   cmd_start ;;
  stop)    cmd_stop ;;
  status)  cmd_status ;;
  restart) cmd_stop; cmd_start ;;
  tail)    cmd_tail ;;
  help|-h|--help) usage 0 ;;
  *)       usage 1 ;;
esac
