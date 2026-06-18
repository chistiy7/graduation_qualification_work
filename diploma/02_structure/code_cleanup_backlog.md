# Сводка: оптимизация и приведение кода к чистому виду

> **Когда заниматься:** после полностью готовой функциональной программы (все сценарии, глава 4, демо для защиты). До этого — только фиксы, блокирующие расчёт или текст диплома.
>
> **Проект:** `lab-test-planner/` + синхронизация с `diploma/01_chapters/`, `docs/`, контекстом.

**Связанные файлы:** `diploma/00_context/state.md`, `diploma/00_context/glossary.md`, `lab-test-planner/docs/architecture.md`, `lab-test-planner/docs/variables_map.md`.

---

## Принципы чистки

1. **Одна истина** — нормативы времени/мощности/стоимости из техкарты (`test_program_catalog`), не дубли в `TestStage` и не захардкоженные таблицы без пометки.
2. **Min C только** — целевая функция = `VariantMetrics::C`; без K, нормировки, вариантов 0/1.
3. **Модель энергии** — `E_work = P_mode × t_program / 60`; деформационная энергия W — контроль/предупреждение, не планирование времени.
4. **Простой** — `t_idle` и η в отчёте; `E_idle = C_idle = 0`; косвенный учёт через `C_area` за `T_cycle`.
5. **Минимальный diff** — удалять legacy-поля и мёртвый код пакетами с прогоном `LabPlanner_test`.

---

## Уже сделано (промежуточная чистка)

| Что | Где |
|-----|-----|
| Убраны `preloadMin`, `unloadMin`, `returnMin` | `test_stage.hpp`, `operation_energy.hpp` |
| Убран КПД стенда `efficiency` (не читался; в энергии был hardcode) | `lab_equipment.hpp`, `stand_catalog.*` |
| Убраны `idlePowerKw`, `idleCostPerHour`, `K_POWER_IDLE`, `K_IDLE_COST` | `lab_equipment.hpp`, `physics_constants.hpp` |
| Убраны K, `ObjectiveMode`, `ObjectiveWeights`, нормировка | `problem.hpp`, `metrics.*`, GUI, JSON, тесты |
| K исключена из контекста диплома | `topic.md`, `glossary.md`, `state.md` (бэклог по главам) |

---

## Бэклог: доменные модели

### `TestStage` (`test_stage.hpp`)

- [ ] **Слить три поля времени** в одно `tProgramMin` (или оставить пару: норматив + занятость, если появится расхождение): сейчас `durationMin` = `durationNormMin` = `cycleTimeMin`.
- [ ] **`costOp`, `laborHours`** — перенести в `TestProgramDef` / техкарту или явно назвать «экономика по умолчанию» в одном месте (`defaultOperationEconomics` → deprecated).
- [ ] **`WorkloadRegime`** — внутренняя метка каталога; не ввод пользователя. Либо убрать из `TestStage`, оставить только `TestKind` (3 значения для W).
- [ ] **Блок W / предупреждение** — `deformationEnergyUsed`, `minLoadTimeMin`, `energyWarning`, hardcoded `efficiency = 0.88` в `operation_energy.cpp`: решить — оставить как dev-warning или удалить вместе со старой формулой σ²V/(2Eη).
- [ ] **`MechanicalTestStage` / `ThermalTestStage`** — пустые наследники; удалить, если нигде не используются полиморфно.

### `LabEquipment` (`lab_equipment.hpp`)

- [ ] **`fundTimeMin` + амортизация** — по `global_fix.md` отложено: привязать `C_amort` к `t_busy(e)` или `T_cycle`, а не к фиксированному фонду (согласовать с научруком).
- [ ] **`nominalPowerKw`** — только переналадка (`P_setup`); комментарии и отчёт согласовать с техкартой (`setupPowerKw` на операции приоритетнее).

### Каталоги

- [ ] **`operationsForRegime()`** — мёртвый код в `test_operation_catalog.cpp`; удалить.
- [ ] **`OperationNorms`** в `operation_energy.hpp` — тонкая обёртка; свернуть с `TestProgramDef` или удалить.
- [ ] **`LabProgramMode` / GUI-пресеты** — после CLI-сценария решить: оставить для демо или заменить на `ScenarioInput` only.

---

## Бэклог: ядро (`engine/`)

- [ ] **`EquipmentUtilization::energySetupKwh` / `costEnergySetup`** — не заполняются в `metrics.cpp`; считать в одном месте или убрать поля.
- [ ] **`VariantMetrics::T`** — алиас `T_sum`; убрать или оставить один показатель в API/CSV.
- [ ] **`ProgramEfficiencyEngine`** vs `MetricsEngine` — частичное дублирование T_sum/T_cycle; объединить или чётко разделить «отчёт» vs «ЦФ».
- [ ] **Два маршрута в `LabOptimiser::plan`** (by specimen / by operation) — ок для MVP; позже один явный алгоритм или документировать как эвристику.
- [ ] **Полный парсер JSON** сценария (`loadScenarioJson`) — сейчас заглушка по имени файла.

---

## Бэклог: документация ПО

Синхронизировать с кодом после чистки:

- [ ] `lab-test-planner/docs/architecture.md` — убрать K, WeightedK, objectiveMode.
- [ ] `lab-test-planner/docs/variables_map.md` — min C only; убрать η как КПД стенда.
- [ ] `lab-test-planner/docs/chapter_03.md`, `uml_current.md`
- [ ] `lab-test-planner/README.md` — актуализировать при необходимости
- [ ] `diploma/02_structure/lab_planner_rebuild_plan.md` — убрать K, variant 0/1, устаревшие фазы

---

## Бэклог: теория диплома (после готового ПО)

- [ ] **Главы 2–3** — убрать K, нормировку, сравнение 0/1; энергия = `P_mode × t_program`, не σ²V/(2Eη)×тариф.
- [ ] **Глава 2 §2.1** — 6 стендов в коде vs 7 строк в тексте; объединение печи/усталости — по фактической номенклатуре.
- [ ] **Запретные зоны** — пересечение = проход, буферы в `C_area` (брифинги / `fix.md`).
- [ ] **`diploma_v1.md` / Word** — синхронизация с `01_chapters/`.
- [ ] **`Фазы_работы_над_дипломом.md`** — практический результат, без K и «сравнить варианты».

---

## Порядок работ (рекомендуемый)

1. Заморозить функционал → все демо-сценарии и глава 4 описывают фактическое ПО.
2. **Доки ПО** — variables_map + architecture (чтобы был эталон перед рефакторингом).
3. **Домен** — `TestStage` (время, costOp), каталог программ.
4. **Ядро** — metrics/utilization/amortization, JSON load.
5. **Теория** — главы 2–3 под финальный код.
6. Прогон тестов + ручная проверка отчётов `output/report_*.txt`.

---

## Чеклист перед закрытием чистки

- [ ] `cmake --build build && ./build/LabPlanner_test` — все тесты зелёные
- [ ] Нет упоминаний `WeightedK`, `ObjectiveMode`, `efficiency` (КПД), `preload*` в `src/`
- [ ] Один источник `t_program` на операцию
- [ ] `glossary.md` и `topic.md` совпадают с кодом
- [ ] Отчёт: `C` = сумма статей, `E_idle` = 0, η ≤ 100%

---

*Последнее обновление: 2026-06-16 — создан документ; зафиксирована промежуточная чистка legacy/K.*
