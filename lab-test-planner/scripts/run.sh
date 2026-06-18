#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

if [[ "${1:-}" == "--gui" ]]; then
  APP="$ROOT/build/lab-test-planner-gui.app"
  if [[ ! -d "$APP" ]]; then
    echo "Соберите проект: ./scripts/build.sh"
    exit 1
  fi
  exec open "$APP"
fi

BIN="${BIN:-$ROOT/build/LabPlanner}"
if [[ ! -x "$BIN" ]]; then
  echo "Соберите проект: ./scripts/build.sh"
  exit 1
fi
exec "$BIN" "$@"
