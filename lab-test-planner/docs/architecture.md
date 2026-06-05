# Архитектура LabPlanner

Соответствие главе 3 диплома: [`chapter_03.md`](chapter_03.md).

Принцип: **все метрики выводятся из входных данных и маршрута**. Числа из учебного примера в §3.6 не зашиты в код.

## Слои (§3.7 диплома)

| Слой | Модули | § гл. 3 |
|------|--------|---------|
| Препроцессор | `Preprocessor` — загрузка, `validate()` | 3.2, 3.5 |
| Ядро | `RoutePlanner`, `RouteAnalyzer`, `StandStateAnalyzer`, `MetricsEngine`, `LabOptimiser` | 3.3–3.5 |
| Постпроцессор | `Postprocessor` → `output/` | 3.6 |
| GUI | `ui/gui_app.cpp` | 3.7 |
| IO | `scenario_json` — save/load входных данных | 3.2 |

## Точки входа

| Команда | Назначение |
|---------|-----------|
| `LabPlanner` | GUI |
| `LabPlanner --cli` | **интерактивный ввод**: площадь, партия, типы испытаний → размещение + расчёт |
| `LabPlanner --debug demo_simple` | отладочный пресет (1 образец) |
| `LabPlanner --debug demo_two_specimens` | отладочный пресет (2 образца) |

Демо-пресеты — только для отладки; основной сценарий — пользовательский ввод.

## Поток расчёта

```
ScenarioInput (площадь, партия, типы испытаний)         ← interactive.cpp
    → buildFromInput → ProblemDefinition (стенды без координат)
    → optimizeLayout → размещение стендов на сетке gridRows×gridCols (min C)
    → Preprocessor::ensureValid()
    → RoutePlanner → TestRoute (вариант 0 и 1)
    → RouteAnalyzer → N, L, занятость, t_set, c_set
    → StandStateAnalyzer → матрица состояний
    → MetricsEngine → T, C (+разложение, вкл. транспорт), η, ЦФ
    → LabOptimiser::compareStrategies()
    → renderLayoutMatrix → карта размещения (0/тип/R)
    → Postprocessor → report_*.txt, comparison.csv
```

## Оптимизатор размещения (`engine/layout_optimizer`)

- Сетка строится из площади: `rows = round(длина/ячейка)`, `cols = round(ширина/ячейка)`.
- Перебор размещений при `P(ячейки, стенды) ≤ 200000`, иначе эвристика (компактная раскладка + локальный поиск).
- Критерий — min **C**; C зависит от позиций через транспортную статью (перемещения образцов).

## Вход: `ScenarioBundle` / `ProblemDefinition`

| Гл. 3 | Поле |
|-------|------|
| S | `specimens` |
| D | `operations` |
| E | `equipment` |
| G, Z | `laboratory` |
| Req, Cap, Pred | `required`, `capable`, `precedence` |
| c_lab, шаг сетки | `laborRatePerHour`, `minutesPerGridStep` |
| Ячейка 2×2 м | `gridCellSizeM` |
| ЦФ | `objectiveMode`, `weights` |

## Выход

- `OptimisationResult`: маршруты 0/1, `ComparisonRow` (T, C, N, L, η, ЦФ, Δ%)
- `output/report_<scenario>.txt`
- `output/comparison.csv`

## Целевая функция (§3.4)

- `ObjectiveMode::TotalCostRub` — min **C** (руб), открытая постановка (по умолчанию).
- `ObjectiveMode::WeightedK` — min **K** = α₁T̃ + α₂C̃ + α₃Ñ + α₄L̃ + α₅(2−η̃).

## Матрица состояний (§3.2)

`StandStateAnalyzer`: свободен → переналадка/занят; проверка Cap и Pred.  
Без состояния «ошибка».

## Сетка (§3.2)

- `gridCellSizeM = 2.0` — ячейка 2×2 м.
- **L** — шаги ячеек (Manhattan).
- **c_cell** — `LabEquipment::cellPlacementCost` (руб/ячейка).

## RouteAnalyzer

- **N** — перенастройки: смена операции на стенде или первый запуск.
- **L** — сумма Manhattan между ячейками стендов по маршруту.
- **t_move** — `L × minutesPerGridStep`.
- **Занятость** — длительности операций + перенастройки.
- **c_set** — `setupCost` на событие перенастройки.

## Варианты 0 / 1

Задаются **стратегией** (`RouteOrderingStrategy`), не числами извне:

- `BySpecimenThenOperation` — вариант 0 (по образцам);
- `ByOperationThenSpecimen` — вариант 1 (группировка по операциям).

## Расширение

| Добавление | Куда |
|------------|------|
| Новый стенд / операция | `program_builder`, каталоги `domain/` |
| t_set(e,d) по операции | `LabEquipment` + map в `RouteAnalyzer` |
| Оптимизатор сетки | `engine/layout/` |
| Полный JSON load | `io/scenario_json.cpp` |

## Демо-сценарии

| ID | Описание |
|----|----------|
| `demo_simple` | 1 образец, BasicMechanical |
| `demo_two_specimens` | 2 образца — структура §3.6, метрики из маршрута |
| `buildDemoForMode(...)` | MechanicalExtended, ThermalCycle, Thermomechanical |

## Тесты

`LabPlanner_test` — свойства на `demo_simple`: N₁≤N₀, L₁≤L₀, согласованность анализаторов, C₁≤C₀ при `TotalCostRub`.
