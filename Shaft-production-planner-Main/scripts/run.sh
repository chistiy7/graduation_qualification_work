#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

BIN="${BIN:-$ROOT/build/Planner}"

if [[ ! -x "$BIN" ]]; then
  echo "Бинарник не найден: $BIN"
  echo "Сначала выполните: ./scripts/build.sh"
  exit 1
fi

if [[ ! -d "$ROOT/resources" ]]; then
  echo "Папка resources/ не найдена в $ROOT"
  exit 1
fi

exec "$BIN"
