#!/usr/bin/env bash
# Build and run the dsky unit test suites.
# Each suite is a standalone cmake project in its own directory; the binary
# is built into <suite>/build and must be the only executable at the build
# dir root. Prints PASS/FAIL per suite; exits non-zero if any suite fails.
#
# Suites are listed explicitly below (not auto-discovered) so that legacy
# demo projects (Font) stay out of the runner until they are converted into
# real tests. Add a suite here when its step lands.
set -u
cd "$(dirname "$0")"

suites=(StatusMachine HmiData Event clockAction ListOfActions)

overall=0
for s in "${suites[@]}"; do
  b="$s/build"
  mkdir -p "$b"
  if cmake -S "$s" -B "$b" >"$b/cmake-configure.log" 2>&1 \
     && cmake --build "$b" -j"$(nproc)" >"$b/build.log" 2>&1; then
    bin=$(find "$b" -maxdepth 1 -type f -executable | head -n1)
    if [ -n "$bin" ] && "$bin" >"$b/run.log" 2>&1; then
      echo "PASS $s"
    else
      echo "FAIL $s (run, see $b/run.log)"
      overall=1
    fi
  else
    echo "FAIL $s (build, see $b/cmake-configure.log or $b/build.log)"
    overall=1
  fi
done

exit $overall
