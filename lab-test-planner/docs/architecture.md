# Архитектура LabPlanner

Соответствие главе 3 диплома: [`chapter_03.md`](chapter_03.md).  
Спецификации правок: [`diploma/agent_promts/переналадка.md`](../diploma/agent_promts/переналадка.md), [`diploma/agent_promts/простой.md`](../diploma/agent_promts/простой.md).

Принцип: **все метрики выводятся из входных данных и маршрута**. Эталон входа — CLI (`ScenarioInput` / `SpecimenGroup` → `buildFromInput`). Демо-пресеты (`--debug`) устарели и **не являются эталоном модели**.

## Слои (§3.7 диплома)

| Слой | Модули | § гл. 3 |
|------|--------|---------|
| Препроцессор | `Preprocessor` — загрузка, `validate()` | 3.2, 3.5 |
| Ядро | `RoutePlanner`, `RouteAnalyzer`, `StandStateAnalyzer`, `MetricsEngine`, `ProgramEfficiencyEngine`, `LabOptimiser`, `layout_optimizer` | 3.3–3.5 |
| Постпроцessor | `Postprocessor` → `output/` | 3.6 |
| Связка | `Pipeline`, `scenario_runner` (`formatScenarioRunOutput`, `runUserScenario`) | 3.7 |
| GUI (Qt 6) | `src/qt/` — `MainWindow`, `InputPage`, `ResultsPage`, `LayoutGridWidget` | 3.7 |
| IO | `scenario_json` — save/load `ScenarioBundle` | 3.2 |

Сборка: статическая библиотека **`lab_core`** + `LabPlanner` (консоль) + `lab-test-planner-gui.app` (Qt, опция `LAB_PLANNER_BUILD_QT_GUI=ON`).

## Точки входа

| Команда / приложение | Назначение |
|----------------------|------------|
| `LabPlanner` (без аргументов) | справка по режимам |
| `LabPlanner --cli` | интерактивный ввод: площадь, партия, группы → `Pipeline::runFromInput` |
| `LabPlanner --debug …` | встроенные пресеты (`demo_simple`, `demo_two_specimens`, `demo_8_types_80`) |
| `lab-test-planner-gui.app` | Qt GUI: те же входы/выходы, что CLI; визуализация сетки и таблицы метрик |

Эталон пользовательского ввода — **`ScenarioInput`** + **`SpecimenGroup`** (CLI `--cli` и вкладка «Задача» в GUI). Демо-пресеты — для отладки и быстрых прогонов.

## Поток расчёта

```
ScenarioInput (площадь, партия, SpecimenGroup[])
    → buildFromInput → ProblemDefinition
    → optimizeLayout → координаты стендов (min C, DirectionalBuffer по ГОСТ §2.2)
    → Preprocessor::ensureValid()
    → LabOptimiser::plan()
         ├─ маршрут «по образцам»
         └─ маршрут «по операциям» → выбор min C
    → RouteAnalyzer → N, L, t_busy/t_idle по стендам, C_energy_setup/idle
    → StandStateAnalyzer → матрица состояний (Cap, Pred)
    → MetricsEngine → T, C, η, разложение CostBreakdown
    → Postprocessor → report_*.txt, plan.csv
    → formatScenarioRunOutput (CLI / GUI)
```

## Qt GUI (`src/qt/`)

GUI **не содержит расчётной логики** — только сбор `ScenarioInput`, вызов `Pipeline` / `runUserScenario` и отображение `PipelineOutput`.

| Компонент | Файл | Роль |
|-----------|------|------|
| `MainWindow` | `mainwindow.cpp` | меню, toolbar, вкладки «Задача» / «Результаты», save/load JSON |
| `InputPage` | `pages/input_page.cpp` | пресеты, параметры `--cli`, таблица групп образцов |
| `ResultsPage` | `pages/results_page.cpp` | вкладки: сводка, C, стенды, маршрут, планировка, полный отчёт |
| `LayoutGridWidget` | `widgets/layout_grid_widget.cpp` | цветная карта сетки (аналог `renderLayoutMatrix`) |

```
InputPage → runUserScenario / Pipeline::run → PipelineOutput
         → ResultsPage::showOutput + LayoutGridWidget
         → formatScenarioRunOutput (вкладка «Полный отчёт»)
```

Файловое меню: `loadScenarioJson` → `Pipeline::run`; `saveScenarioJson`; пути `reportPath` / `csvPath` из `Postprocessor`.

## Вход: `ScenarioBundle` / `ProblemDefinition`

| Гл. 3 | Поле в коде |
|-------|-------------|
| S | `specimens` |
| D | `operations` (`TestStage`) |
| E | `equipment` (`LabEquipment`) |
| G, Z | `laboratory` |
| Req, Cap, Pred | `required`, `capable`, `precedence` |
| c_lab | `laborRatePerHour` |
| Тариф электроэнергии | `electricityTariffPerKwh` |
| Ячейка 2×2 м | `gridCellSizeM` |
| Шаг перемещения | `minutesPerGridStep` |
| Фонд времени стенда | `LabEquipment::fundTimeMin` — **только амортизация**, не для η/простоя |
| Аренда площади | `rentRatePerM2Hour` (0 → `RENT_RATE_DEFAULT_RUB_M2_HOUR`) |
| ЦФ | `objectiveMode`, `weights` |

**Правило партии:** на образец — ровно один `TestOperationType` (через `SpecimenGroup`); комбинированные режимы — один тип операции (`TensionPlusTorsion`, `Thermomechanical` и т.д.).

## Выход

- `PipelineOutput`: `ScenarioBundle` + `PlanResult` + `reportPath`, `csvPath`, `layoutNote`, `layoutEvaluated`, `elapsedMs`
- `PlanResult`: `TestRoute` + `VariantMetrics` (T_sum, T_cycle, C, N, L, η_avg, bottleneck, `CostBreakdown`, `equipmentStats`, `ProgramEfficiencyMetrics`)
- `output/report_<scenario>.txt` — разложение C, простой по стендам, маршрут, карта (ASCII)
- `output/plan.csv` — агрегированные метрики и статьи C
- Qt GUI — те же данные в таблицах + `LayoutGridWidget` + текст `formatScenarioRunOutput`

## Целевая функция (§3.4)

| Режим | Критерий |
|-------|----------|
| `TotalCostRub` (по умолчанию) | **min C** — сумма статей `CostBreakdown` |
| `WeightedK` | **min K** = α₁T̃ + α₂C̃ + α₃Ñ + α₄L̃ + α₅(2−η̃) |

## Себестоимость C (`CostBreakdown`)

Итог: `C = cost.total()` → `VariantMetrics::C` → `K` при `TotalCostRub`.

| Статья в коде | Смысл | Модуль |
|---------------|-------|--------|
| `prepLabor` | труд подготовки образцов | `metrics.cpp` |
| `operationMaterials` | материалы испытаний (`costOp`) | `metrics.cpp` |
| `energyWork` | **C_energy_work** (`costEnergy` испытаний) | `metrics.cpp`, `operation_energy.cpp` |
| `operationLabor` | труд по операциям | `metrics.cpp` |
| `setup` | **C_setup** — рубли переналадки | `route_analysis.cpp` |
| `energySetup` | **C_energy_setup** — `P_setup × t_setup × тариф` | `route_analysis.cpp` |
| `amortization` | **C_amort** — `amortPerHour × fundTimeMin/60` (без изменений) | `metrics.cpp` |
| `transport` | перемещения: `moveTimeMin × c_lab` | `metrics.cpp` |
| `area` | **C_area** — занятая площадь × аренда × T_cycle | `layout_area.cpp`, `metrics.cpp` |

Константы мощности: `physics_constants.hpp` — `K_POWER_SETUP` (0,2), `RENT_RATE_DEFAULT_RUB_M2_HOUR`.

**Не входят в C:** `C_idle`, `C_energy_idle` (в отчёте = 0).

## Три состояния стенда

| Состояние | Учёт во времени | Учёт в C |
|-----------|----------------|----------|
| **Испытание** | `t_test` → `t_busy` | `operationMaterials`, `energyWork` |
| **Переналадка** | `t_setup` → `t_busy`, **T_sum** | `setup`, `energySetup`; **не простой** |
| **Простой** | `t_idle = T_cycle − t_busy` (аналитика) | не штрафуется; косвенно через **C_area** за T_cycle |

Переналадка и испытание увеличивают занятость; простой — остаток фонда без работы.

## Переналадка стендов (N)

**Реализация:** `route_analysis.cpp`, `stand_state_matrix.cpp`.

**Режим** стенда = `operationId` шага маршрута (растяжение, `tension_torsion`, `thermal_static` и т.д.).

### Когда начисляется переналадка

1. Стенд используется **впервые** в маршруте (первичная подготовка).
2. Стенд уже использовался, но `operationId` **изменился** (смена режима).

**N** — число таких событий (не число уникальных стендов).

### Когда переналадки нет

- тот же `operationId` на том же стенде (следующий образец, тот же режим);
- перемещение между ячейками (**L**, транспорт);
- подготовка образца до стенда (`prepTimeMin` — отдельное слагаемое **T**).

### Примеры смены режима (+N)

`tension` → `compression`; `tension_torsion` → `tension`; `thermal_static` → `thermo_mech` на `e_furnace`.

### Алгоритм (псевдокод)

```
mode[e] ← не задан
для каждого step (specimen, opId, stand e):
    если mode[e] не задан или mode[e] ≠ opId:
        N += 1;  t_setup += e.setupTimeMin;  c_setup += e.setupCost
        C_energy_setup += P_setup(e) × t_setup_event × тариф
        t_busy[e] += e.setupTimeMin
        mode[e] ← opId
    t_test[e] += длительность_операции
    t_busy[e] += длительность_операции
```

`P_setup = K_POWER_SETUP × P_nom` (или `LabEquipment::nominalPowerKw`).

### Влияние на показатели (событие переналадки)

| Показатель | Изменение |
|------------|-----------|
| **N** | +1 |
| **T** | + `setupTimeMin` |
| **C** (`setup`) | + `setupCost` |
| **C** (`energySetup`) | + энергия переналадки |
| **t_busy** | + `setupTimeMin` |
| **η** | пересчёт `t_busy / T_cycle` (≤ 100%) |

### Матрица состояний

`StandStateAnalyzer`: для каждого шага — `setupPerformed`, если режим сменился или стенд новый; `setupCount` ≡ `RouteAnalyzer::setupCount`.

### Связь с оптимизатором маршрута

`LabOptimiser` сравнивает:

- `BySpecimenThenOperation` — по образцам;
- `ByOperationThenSpecimen` — группировка по операциям.

**Группировка одинаковых режимов** на одном стенде снижает **N**, **T** и часто **C** (меньше переналадок и `C_setup`).

Пример: порядок `T, T, T+K, T+K` лучше, чем чередование `T, T+K, T, T+K`.

## Время партии и загрузка (η)

**Реализация:** `metrics.cpp`, `route_analysis.cpp`.

```
T_sum   = prep + Σ t_test + Σ t_setup + t_move     // суммарная трудоёмкость
T_cycle = max_e(t_busy_e)                           // цикл партии (параллель)
t_busy  = t_test + t_setup
t_idle  = T_cycle − t_busy                          // при T_cycle > 0
η_e     = t_busy / T_cycle                          // доля 0..1, в отчёте %
η_avg   = среднее η_e
bottleneck = стенд с max(t_busy)
```

**C_area** (`layout_area.cpp`):

```
used_cells = equipment_cells + reserved_cells   // Stand + Buffer/Forbidden
C_area = used_cells × cell_area_m² × rent_rate × (T_cycle / 60)
```

### Отчёт по стендам (`EquipmentUtilization`)

| Поле | Описание |
|------|----------|
| `t_test`, `t_setup`, `t_busy`, `t_idle` | минуты |
| `utilization` | η_e (доля) |
| `costSetup`, `costAmort` | руб |
| `E_idle`, `C_idle`, `C_energy_idle` | в отчёте **0** |

Сводка: `T_sum`, `T_cycle`, `totalIdleMin`, `η_avg` (%), `bottleneck`, агрегированный маршрут.

## Эффективность программы (`program_efficiency.cpp`)

**Реализация:** `ProgramEfficiencyEngine` → `PlanResult::efficiency`.

Разбивка **времени работы программы** (гл. 2, §2.4):

```
workTimeMin (T_sum) = prepMin + testMin + setupMin + moveMin
cycleMin   (T_cycle) = max_e(t_busy_e)
parallelismIndex     = T_cycle / T_sum   // 0..1
```

| Поле | Смысл |
|------|--------|
| `prepMin` | подготовка всех образцов партии |
| `testMin` | суммарное время испытаний на стендах |
| `setupMin` | переналадки |
| `moveMin` | перемещения между ячейками |
| `workTimeMin` | полное время работы программы |
| `cycleMin` | фактический цикл при параллельной работе стендов |

В отчёте — блок «Эффективность программы (время)»; в CSV — `program_work_time_min`, `program_time_*`.

## Оптимизатор размещения (`layout_optimizer`)

- Сетка из площади помещения; шаг ячейки 2×2 м.
- **Зоны безопасности** — `LabEquipment::buffer` (`DirectionalBuffer`: front/back/side) из `stand_catalog.cpp` по §2.2 (ГОСТ). Проверка: `layout_optimizer.cpp` → `inForbiddenZone()`, ориентация `autoOrient`. Не «1 ячейка по периметру».
- Критерий — min **C** (включая **C_area** за T_cycle и амортизацию за фонд).

## Сетка и маршрут

- **L** — сумма Manhattan между ячейками стендов по порядку шагов.
- **t_move** = `L × minutesPerGridStep` → `cost.transport`.
- **C_area** — аренда занятых ячеек (стенды + зоны X) за **T_cycle** (`layout_area.cpp`).

## Варианты порядка маршрута

| Стратегия | Смысл |
|-----------|--------|
| `BySpecimenThenOperation` | все операции образца подряд |
| `ByOperationThenSpecimen` | группировка по типу операции |

Выбирается вариант с меньшей **C** (`LabOptimiser::plan`).

## Тесты (`LabPlanner_test`)

| Тест | Проверка |
|------|----------|
| `testSetupOnModeChange` | N при группировке режимов < при чередовании |
| `testCycleTimeAndUtilization` | T_cycle = max(t_busy), η ≤ 100%, C_area > 0 |
| свойства плана | согласованность `RouteAnalyzer` / `StandStateAnalyzer` / `MetricsEngine` |

## Расширение

| Добавление | Куда |
|------------|------|
| Новый стенд / операция | `stand_catalog`, `test_operation_catalog`, `buildFromInput` |
| t_horizon = T_total (вариант 2) | `RouteAnalyzer` + флаг в `ProblemDefinition` |
| t_set по операции | `LabEquipment` + map в `RouteAnalyzer` |
| Полный JSON load | `io/scenario_json.cpp` |
| Новый экран GUI | `src/qt/pages/`, данные только из `PipelineOutput` |
| Фоновый расчёт в GUI | `QtConcurrent` / `QThread` вокруг `Pipeline::run` |
