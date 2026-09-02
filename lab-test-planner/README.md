# LabPlanner — планировщик испытательной лаборатории

Метрики **T, C, N, L, η** и целевая функция (**C** в руб. или **K**) считаются из данных задачи и построенного маршрута.  
Числа учебного примера §3.6 в дипломе **не захардкожены**.

## Документация

| Файл | Содержание |
|------|------------|
| [`docs/chapter_03.md`](docs/chapter_03.md) | Глава 3: формулы, входы/выходы, соответствие текст ↔ код |
| [`docs/architecture.md`](docs/architecture.md) | Архитектура (§3.7), поток расчёта, GUI |
| [`docs/uml_current.md`](docs/uml_current.md) | UML-классы и поток данных (Mermaid) |
| [`docs/variables_map.md`](docs/variables_map.md) | Таблица переменных: теория → код |
| [`docs/test_operation_catalog.md`](docs/test_operation_catalog.md) | Каталог операций и стендов (гл. 1–2) |

## Архитектура

```
ScenarioInput / ScenarioBundle
    → Pipeline (размещение → препроцессор → LabOptimiser → постпроцessor)
    → PipelineOutput
    → CLI / Qt GUI (отображение)
```

Библиотека **`lab_core`** — ядро без UI. Два исполняемых файла:

| Бинарник | Назначение |
|----------|------------|
| `LabPlanner` | Консоль: `--cli`, `--debug` |
| `lab-test-planner-gui.app` | Qt 6 GUI (macOS) |

## Сборка

```bash
./scripts/build.sh
```

Требования: **CMake ≥ 3.16**, **C++20**, **Qt 6 Widgets** (Homebrew: `brew install qt6`).

Ручная сборка:

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_PREFIX_PATH=$(brew --prefix qt6)
make -j4
macdeployqt lab-test-planner-gui.app -dmg   # опционально: .dmg для macOS
```

## Запуск

### Консоль

```bash
./build/LabPlanner --cli                        # интерактивный ввод
./build/LabPlanner --debug demo_simple          # пресет 2 образца
./build/LabPlanner --debug demo_two_specimens
./build/LabPlanner --debug demo_8_types_80      # 80 образцов, 8 видов × 10
./scripts/run.sh --cli                          # обёртка над LabPlanner
```

### Qt GUI

```bash
open build/lab-test-planner-gui.app
# или
./scripts/run.sh --gui
```

GUI повторяет CLI: вкладка **«Задача»** (параметры + группы образцов), **«Результаты»** (сводка, C, стенды, маршрут, карта сетки, полный отчёт). Меню: открыть/сохранить JSON, пути отчёта и CSV. Логика расчёта — только в `lab_core`; GUI не содержит формул.

## Тесты

```bash
./build/LabPlanner_test
```

Проверяются свойства модели (N, L, C, матрица состояний, pipeline + layout), не фиксированные числа из §3.6.

## Данные

- **Вход:** `ScenarioInput` (CLI/GUI) или `ScenarioBundle` (JSON, пресеты).
- **Выход:** один оптимальный план — размещение стендов + порядок операций с min **C**.
- **Файлы:** `output/report_*.txt`, `output/plan.csv`, `data/scenarios/*.json`.
