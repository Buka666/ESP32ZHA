#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
LOG_FILE="$ROOT_DIR/build_codex.log"

find_idf_export() {
  local candidates=(
    "${IDF_PATH:-}/export.sh"
    "/opt/esp/idf/export.sh"
    "$HOME/esp/idf/export.sh"
    "/workspace/esp-idf/export.sh"
  )

  for c in "${candidates[@]}"; do
    [[ -n "$c" && -f "$c" ]] && { echo "$c"; return 0; }
  done
  return 1
}

if ! command -v idf.py >/dev/null 2>&1; then
  if export_sh="$(find_idf_export)"; then
    # shellcheck disable=SC1090
    . "$export_sh"
  fi
fi

if ! command -v idf.py >/dev/null 2>&1; then
  echo "SKIP: idf.py not found. Install/export ESP-IDF to run full compile check."
  exit 0
fi

cd "$ROOT_DIR"

set +e
idf.py set-target esp32c6 >"$LOG_FILE" 2>&1
idf.py -D CMAKE_BUILD_TYPE=Release build >>"$LOG_FILE" 2>&1
status=$?
set -e

if [[ $status -eq 0 ]]; then
  echo "Compile check passed"
  exit 0
fi

echo "Initial build failed, running automatic Zigbee fixes..."
python3 "$ROOT_DIR/tools/auto_fix_zigbee_errors.py" "$LOG_FILE" "$ROOT_DIR/main/zigbee_light.c"

set +e
idf.py -D CMAKE_BUILD_TYPE=Release build >>"$LOG_FILE" 2>&1
status=$?
set -e

if [[ $status -eq 0 ]]; then
  echo "Compile check passed after auto-fix"
  exit 0
fi

echo "Build still failing. See $LOG_FILE"
exit $status
