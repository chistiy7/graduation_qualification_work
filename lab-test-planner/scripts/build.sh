#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
mkdir -p build
cd build
QT_PREFIX="$(brew --prefix qt6 2>/dev/null || true)"
CMAKE_EXTRA=()
if [[ -n "$QT_PREFIX" ]]; then
  CMAKE_EXTRA+=(-DCMAKE_PREFIX_PATH="$QT_PREFIX")
fi
cmake .. -DCMAKE_BUILD_TYPE=Release "${CMAKE_EXTRA[@]}"
cmake --build . -j"$(sysctl -n hw.ncpu 2>/dev/null || echo 4)"
ln -sf build/compile_commands.json "$ROOT/compile_commands.json" 2>/dev/null || true
echo "Готово:"
echo "  CLI:  $ROOT/build/LabPlanner --cli"
echo "  GUI:  open $ROOT/build/lab-test-planner-gui.app"
