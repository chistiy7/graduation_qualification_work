#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
BIN="${BIN:-$ROOT/build/LabPlanner}"
if [[ ! -x "$BIN" ]]; then
  echo "Соберите проект: ./scripts/build.sh"
  exit 1
fi
exec "$BIN" "$@"
