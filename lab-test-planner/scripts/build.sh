#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
mkdir -p build resources
if [[ ! -f resources/Arial.ttf ]]; then
  SRC="../Shaft-production-planner-Main/resources/Arial.ttf"
  if [[ -f "$SRC" ]]; then cp "$SRC" resources/Arial.ttf; fi
fi
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j"$(sysctl -n hw.ncpu 2>/dev/null || echo 4)"
ln -sf build/compile_commands.json "$ROOT/compile_commands.json" 2>/dev/null || true
echo "Готово: $ROOT/build/LabPlanner"
