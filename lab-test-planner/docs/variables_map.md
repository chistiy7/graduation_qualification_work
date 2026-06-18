# Таблица переменных модели: теория → код

Дата: 2026-06-16. Источники: гл. 2–3 (`diploma/01_chapters/`), заголовки `src/**/*.hpp`, `metrics.cpp`, `route_analysis.cpp`, `layout_area.cpp`, `operation_energy.cpp`, `program_efficiency.cpp`.

Принцип: **цель работы** — снижение **C** за счёт оптимизации **планировки**; **целевая функция** — min **C** (`TotalCostRub`) или min **K** (`WeightedK`). На образец — **ровно одно** испытание (`scenario_validation`, `SpecimenGroup`).

---

## Основные переменные (гл. 3)

| Переменная (теория) | Поле / выражение в коде | Класс / файл | Статус |
|---|---|---|---|
| **S** — партия образцов | `ProblemDefinition::specimens` | `problem.hpp` | ✅ |
| **D** — операции | `ProblemDefinition::operations` (`TestStage`) | `problem.hpp` | ✅ |
| **E** — стенды | `ProblemDefinition::equipment` (`LabEquipment`) | `problem.hpp` | ✅ |
| **Z** — вспомогательные зоны | `Laboratory` (ячейки `Buffer`, `Cooling`, …) | `laboratory.hpp` | ✅ |
| **G** — сетка | `ProblemDefinition::laboratory`, `gridRows`, `gridCols` | `problem.hpp` | ✅ |
| **Req(s, d)** | `ProblemDefinition::required[sId][opId]` | `problem.hpp` | ✅ |
| **Cap(e, d)** | `ProblemDefinition::capable[eqId][opId]` | `problem.hpp` | ✅ |
| **Pred(d₁, d₂)** | `ProblemDefinition::precedence` | `problem.hpp` | ✅ |
| **V(s)** — объём образца, м³ | `Specimen::volumeM3`, `ScenarioInput::specimenVolumeM3` | `specimen.hpp`, `scenario_input.hpp` | ✅ |
| **T_sum** — суммарная трудоёмкость, мин | `VariantMetrics::T_sum` = prep + op + setup + move | `metrics.cpp` | ✅ |
| **T_cycle** — календарный цикл, мин | `VariantMetrics::T_cycle` = max_e `t_busy(e)` | `metrics.cpp` | ✅ |
| **T** (совместимость) | `VariantMetrics::T` = `T_sum` | `metrics.cpp` | ✅ |
| **C** — себестоимость, руб | `VariantMetrics::C` = `CostBreakdown::total()` | `metrics.cpp` | ✅ |
| **N** — перенастройки | `VariantMetrics::N` = `RouteAnalysis::setupCount` | `route_analysis.cpp` | ✅ |
| **L** — длина маршрута, шаги | `VariantMetrics::L` = `RouteAnalysis::routeLengthSteps` | `route_analysis.cpp` | ✅ |
| **η** — средняя загрузка | `VariantMetrics::etaAvg` = avg(`t_busy / T_cycle`) | `metrics.cpp` | ✅ |
| **bottleneck** | `VariantMetrics::bottleneckEquipmentId` | `metrics.cpp` | ✅ |
| **K** — критерий | `VariantMetrics::K` = C или взвешенная сумма | `metrics.cpp` | ✅ |
| **c_lab** — ставка труда, руб/ч | `ProblemDefinition::laborRatePerHour` | `problem.hpp` | ✅ |
| **тариф** — электроэнергия, руб/кВт·ч | `ProblemDefinition::electricityTariffPerKwh` | `problem.hpp` | ✅ |
| **rent** — аренда, руб/(м²·ч) | `ProblemDefinition::rentRatePerM2Hour` (0 → `RENT_RATE_DEFAULT_RUB_M2_HOUR`) | `problem.hpp`, `physics_constants.hpp` | ✅ |
| **cell_area** — площадь ячейки, м² | `gridCellSizeM²` = `gridCellSizeM`² | `problem.hpp` | ✅ |
| **C_area** | `CostBreakdown::area` = `areaOccupancyCost(problem, T_cycle)` | `layout_area.cpp` | ✅ |
| **t_move** | `RouteAnalysis::moveTimeMin` = L × `minutesPerGridStep` | `route_analysis.cpp` | ✅ |
| **буфер стенда** | `LabEquipment::buffer` (`DirectionalBuffer`: front/back/side) | `lab_equipment.hpp`, `stand_catalog.hpp` | ✅ |
| **η(d), σ, ΔT** — паспорт стенда | `LabEquipment::efficiency`, `TestStage::sigmaMpa`, `deltaT_C` | `lab_equipment.hpp`, `test_stage.hpp` | ✅ |
| **E, ρ, c_p** — константы | `E_STEEL_GPA`, `RHO_STEEL_KG_M3`, `CP_STEEL_J_KG_K` | `physics_constants.hpp` | ✅ |

### Устарело / не используется в C и η

| Было в теории / старом коде | Сейчас |
|---|---|
| **c_cell** — разовая стоимость ячейки | заменено на **C_area** = rent × occupied_area × T_cycle |
| **η** от `fundTimeMin` | **η_e** = `t_busy / T_cycle` |
| **C_idle**, **C_energy_idle**, штраф простоя | не входят в C; `t_idle` только в отчёте |
| `LabEquipment::cellPlacementCost` | поле есть, в **C** не суммируется (legacy) |
| `idleCostPerHour`, `idlePowerKw` | поля есть, в расчёте C **не** применяются |

---

## Составляющие себестоимости C

`C = prepLabor + operationMaterials + energyWork + operationLabor + setup + energySetup + amortization + transport + area`

| Статья (теория) | Поле `CostBreakdown` | Выражение | Модуль |
|---|---|---|---|
| Труд подготовки | `prepLabor` | Σ `s.prepLaborHours × c_lab` | `metrics.cpp` |
| Материалы испытаний | `operationMaterials` | Σ `op.costOp` по маршруту | `metrics.cpp` |
| Электроэнергия испытаний | `energyWork` | Σ `op.costEnergy` | `operation_energy.cpp` |
| Труд по операциям | `operationLabor` | Σ `op.laborHours × c_lab` | `metrics.cpp` |
| Перенастройки (руб) | `setup` | Σ `eq.setupCost` при смене режима | `route_analysis.cpp` |
| Электроэнергия переналадки | `energySetup` | `K_POWER_SETUP × P_nom × t_setup/60 × тариф` | `route_analysis.cpp` |
| Амортизация | `amortization` | Σ `amortPerHour × fundTimeMin/60` | `metrics.cpp` |
| Транспорт | `transport` | `(moveTimeMin/60) × c_lab` | `metrics.cpp` |
| Занятость площади | `area` (**C_area**) | `N_used_cells × cell_area × rent × (T_cycle/60)` | `layout_area.cpp` |

**C_energy_work** (`operation_energy.cpp`): `costEnergy = energyKwh × tariff`, где `energyKwh = P_nom × (cycleTimeMin/60)`; `cycleTimeMin` может увеличиваться при физическом пределе нагружения (σ, V, η).

---

## Время: T_sum и T_cycle

| Слагаемое | Выражение в коде | Поля |
|---|---|---|
| Подготовка | Σ `Specimen::prepTimeMin` | `prepTimeMin` (CLI: 10 мин) |
| Испытания | Σ `op.cycleTimeMin` (или `durationMin`) по шагам маршрута | `TestStage`, `operation_energy` |
| Перенастройки | `RouteAnalysis::setupTimeMin` | `LabEquipment::setupTimeMin` |
| Перемещения | `RouteAnalysis::moveTimeMin` | L, `minutesPerGridStep` |
| **T_sum** | сумма четырёх слагаемых | `VariantMetrics::T_sum` |
| **t_busy(e)** | test + setup на стенде e | `RouteAnalysis::busyMinutesByEquipment` |
| **T_cycle** | max_e `t_busy(e)` | `VariantMetrics::T_cycle` |
| **t_idle(e)** | `T_cycle − t_busy(e)` (аналитика) | `EquipmentUtilization::idleMin` |
| **η_e** | `t_busy / T_cycle` | `EquipmentUtilization::utilization` |

Блок отчёта «Эффективность программы»: `ProgramEfficiencyEngine` → `ProgramTimeMetrics` (`prepMin`, `testMin`, `setupMin`, `moveMin`, `workTimeMin`≡T_sum, `cycleMin`≡T_cycle, `parallelismIndex`).

---

## Перенастройки N

Событие переналадки (`route_analysis.cpp`):

1. первое использование стенда в маршруте;
2. смена `operationId` на том же стенде.

Не увеличивает N: тот же режим на том же стенде; перемещение между ячейками (только L, transport).

---

## Виды испытаний на выбор (CLI `--cli`)

`selectableTestTypes()` в `scenario_input.cpp`:

| № | ID | `TestOperationType` |
|---|---|---|
| 1 | `tension` | Tension |
| 2 | `torsion` | Torsion |
| 3 | `bending` | Bending |
| 4 | `compression` | Compression |
| 5 | `fatigue` | Fatigue |
| 6 | `tension_torsion` | TensionPlusTorsion |
| 7 | `bending_torsion` | BendingPlusTorsion |
| 8 | `thermo_mech` | Thermomechanical |

**Не в меню выбора** (но в каталоге / режимах программы): `thermal_static`, `thermal_cyclic`, `induction`, `cooling` — только в составе `Thermomechanical`; `compression_bending` — **удалён** (не в объёме валов).

**Режимы `LabProgramMode`:** `BasicMechanical`, `MechanicalExtended`, `Thermomechanical`; `ThermalCycle` — устаревший алиас → `Thermomechanical`.

---

## Взвешенная ЦФ K (`ObjectiveMode::WeightedK`)

| Нормированная | Поле | Формула (теория) |
|---|---|---|
| **T̃** | `VariantMetrics::Tn` | T_sum / T_sum,₀ |
| **C̃** | `VariantMetrics::Cn` | C / C₀ |
| **Ñ** | `VariantMetrics::Nn` | N / N₀ |
| **L̃** | `VariantMetrics::Ln` | L / L₀ |
| **η̃** | `VariantMetrics::EtaN` | η_avg / η₀ |
| α₁…α₅ | `ObjectiveWeights` | default 0.25 / 0.35 / 0.20 / 0.10 / 0.10 |

`K = α₁T̃ + α₂C̃ + α₃Ñ + α₄L̃ + α₅(2 − η̃)`.

По умолчанию: `ObjectiveMode::TotalCostRub` → **K = C**, нормировка не применяется. ПО выдаёт **один** оптимальный план; индекс «0» — только для теоретической нормировки.

---

## Вход CLI (`ScenarioInput`)

| Параметр | Поле | Примечание |
|---|---|---|
| Площадь помещения | `roomAreaM2` | → число ячеек сетки |
| Размер ячейки | `cellSizeM` | 2 м |
| Партия | `batchSize` | = Σ `SpecimenGroup::count` |
| Группы | `groups[]` | `{count, testType}` — один тип на группу |
| V образца | `specimenVolumeM3` | для C_energy |
| c_lab, тариф | `laborRatePerHour`, `energyTariffPerKwh` | |

Сборка: `buildFromInput` → `optimizeLayout` → `LabOptimiser::plan` → `MetricsEngine::compute`.

---

## Выход (`PlanResult` / отчёт)

| Показатель | CSV / отчёт |
|---|---|
| T_sum, T_cycle | `T_sum_min`, `T_cycle_min` |
| C, N, L, η | `cost_*`, `N`, `L`, `eta_avg` |
| bottleneck | текст в `formatReport` |
| C_area, transport | `cost_area`, `cost_transport` |
| Простой по стендам | `t_idle`, без штрафа в C |
| Маршрут | `plan.csv`, агрегированный маршрут в отчёте |

---

## Частично / на будущее

| Тема | Ситуация | Заметка |
|---|---|---|
| Нормировка K с базой 0 | в ПО нет парного отчёта «вариант 0/1» | эталон задаётся вручную при сравнении |
| `t_prep(s)` по типу испытания | константа 10 мин | можно дифференцировать по `TestOperationType` |
| Буфер в оптимизаторе | упрощённо 1 ячейка Чебышёва | в каталоге — направленный буфер по ГОСТ |
| `cellPlacementCost` | не в C | удалить поле или использовать явно |
| §3.7 в дипломе | архитектура в `architecture.md` | синхронизировать с чистовой гл. 3 |
