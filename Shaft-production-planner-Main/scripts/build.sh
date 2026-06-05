#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

if ! command -v cmake >/dev/null; then
  echo "Установите CMake: brew install cmake"
  exit 1
fi

if [[ "$(uname -s)" == "Darwin" ]]; then
  if ! brew list sfml &>/dev/null; then
    echo "Установите SFML 3: brew install sfml"
    exit 1
  fi
fi

BUILD_DIR="${BUILD_DIR:-build}"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j"$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)"

echo ""
echo "Сборка завершена: $ROOT/$BUILD_DIR/Planner"
echo "Запуск: cd \"$ROOT\" && ./scripts/run.sh"
