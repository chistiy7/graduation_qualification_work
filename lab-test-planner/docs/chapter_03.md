# Глава 3 — соответствие текста диплома и LabPlanner

Текст: [`diploma/01_chapters/chapter_03.md`](../../diploma/01_chapters/chapter_03.md).  
Термины: [`diploma/00_context/glossary.md`](../../diploma/00_context/glossary.md).  
Брифинг 28.05: [`diploma/03_capture/sessions/20260528_191654_созвон/context.md`](../../diploma/03_capture/sessions/20260528_191654_созвон/context.md).

**Принцип:** все метрики вычисляются из входных данных и маршрута. Числа §3.6 в тексте — иллюстрация; в коде **не зашиты**.

---

## Структура главы 3

| § | Тема | Статус в тексте | Реализация в ПО |
|---|------|-----------------|----------------|
| 3.1 | Назначение модели | готово | `Pipeline`, `LabOptimiser` |
| 3.2 | Входные данные S, D, E, Z, G | готово | `ProblemDefinition`, `ScenarioBundle` |
| 3.3 | Частные показатели T, C, N, L, η | готово | `MetricsEngine`, `RouteAnalyzer` |
| 3.4 | Целевая функция (C или K) | готово | `ObjectiveMode`, `MetricsEngine::compare` |
| 3.5 | Ограничения, min ЦФ | готово | `Preprocessor::validate`, `StandStateAnalyzer` |
| 3.6 | Пример на мок-данных | черновик | `demo_two_specimens` — **свойства**, не фикс. 174/7566 |
| 3.7 | Программная реализация | **в тексте нет** | см. [`architecture.md`](architecture.md) |

---

## §3.2 — входные данные

| Обозначение (гл. 3) | Смысл | Код |
|---------------------|-------|-----|
| **S** | Партия образцов | `ProblemDefinition::specimens` (`Specimen`) |
| **D** | Испытательные операции | `ProblemDefinition::operations` (`TestStage`) |
| **E** | Стенды | `ProblemDefinition::equipment` (`LabEquipment`) |
| **Z** | Вспомогательные зоны | `Laboratory` + `LabCellKind::Buffer` |
| **G** | Ячейки сетки | `Laboratory::cells()` |
| **Req(s,d)** | Матрица требований | `ProblemDefinition::required` |
| **Cap(e,d)** | Допустимость операции на стенде | `ProblemDefinition::capable` |
| **Pred(d₁,d₂)** | Предшествование | `ProblemDefinition::precedence` |
| **t_op(d)** | Время операции (мин) | `TestStage::durationMin` |
| **t_set(e)** | Перенастройка (мин) | `LabEquipment::setupTimeMin` |
| **t_prep(s)** | Подготовка образца | `Specimen::prepTimeMin` |
| **t_move** | Перемещение | `L × minutesPerGridStep` |
| **c_op, c_en** | Материалы + энергия | `TestStage::costOp`, `costEnergy` |
| **c_set** | Перенастройка (руб) | `LabEquipment::setupCost` |
| **c_lab** | Ставка труда | `ProblemDefinition::laborRatePerHour` |
| **h(d)** | Труд (ч), вкл. обработку результатов | `TestStage::laborHours` + `Specimen::prepLaborHours` |
| **c_am(e)** | Амортизация | `LabEquipment::amortPerHour` |
| **c_cell(e)** | Стоимость ячейки стенда | `LabEquipment::cellPlacementCost` |
| Ячейка **2×2 м** | Штучная модель | `ProblemDefinition::gridCellSizeM = 2.0` |

### Матрица состояний (§3.2, брифинг)

Не «матрица переходов». Состояния стенда по маршруту:

| Состояние | Описание |
|-----------|----------|
| Свободен | Стенд ещё не использовался в маршруте |
| Занят | Выполняется операция |
| Требуется переналадка | Смена типа операции на стенде |

Код: `StandStateAnalyzer` → `StandStateMatrix` (`src/engine/stand_state_matrix.*`).  
Состояние «ошибка» не моделируется — только нормативный процесс.

---

## §3.3–3.4 — расчёт показателей

### Формулы → код

```
T = Σ t_prep + Σ t_op + Σ t_set + L × minutesPerGridStep
    MetricsEngine::compute() + RouteAnalyzer::analyze()

C = Σ(c_op+c_en) + Σ h·c_lab + Σ c_set + Σ c_am·t_busy + Σ c_cell
    MetricsEngine (+ cellPlacementCost по задействованным стендам)

N = число событий перенастройки
    RouteAnalyzer::setupCount ≡ StandStateMatrix::setupCount

L = Σ Manhattan(ячейкаᵢ → ячейкаᵢ₊₁)
    RouteAnalyzer::routeLengthSteps

η_sr = среднее(t_busy / t_fund) по стендам
    MetricsEngine::averageLoad()
```

### Целевая функция (§3.4)

| Режим | `ObjectiveMode` | Критерий min | Когда использовать |
|-------|-----------------|--------------|-------------------|
| Открытая (брифинг) | `TotalCostRub` | **C** (руб), без нормировки | По умолчанию в демо |
| Взвешенная (гл. 2) | `WeightedK` | **K** = α₁T̃ + α₂C̃ + … | Сравнение с гл. 2 |

**Цель работы** (сокращение времени/затрат) ≠ **целевая функция** (C или K).

Нормировка и K: `MetricsEngine::compare()` только при `WeightedK`.  
В отчёте колонка «ЦФ»; в CSV — `objective`.

---

## §3.5 — ограничения

| Ограничение | Проверка |
|-------------|----------|
| Req(s,d) | `RoutePlanner` + `Preprocessor::validate` |
| Cap(e,d) | `Preprocessor::hasCapableEquipment`, матрица состояний |
| Pred(d₁,d₂) | `StandStateAnalyzer::validatePrecedence` |
| Один стенд — одна операция | Модель маршрута (последовательные шаги) |
| Запретные ячейки, Incomp | `Laboratory` (расширение — layout-оптимизатор) |

---

## §3.6 — пример и верификация

Текстовый пример (2 образца, растяжение + кручение) иллюстрирует постановку min C и при необходимости — взвешенную K.

В ПО:

| Сценарий | Назначение |
|----------|------------|
| `demo_simple` | 1 образец, BasicMechanical |
| `demo_two_specimens` | 2 образца — структура как в §3.6, **числа из маршрута** |

```bash
./scripts/run.sh --cli demo_two_specimens
./build/LabPlanner_test   # свойства: N₁≤N₀, L₁≤L₀, C₁≤C₀ (TotalCostRub)
```

**Не делать:** тест на совпадение с T₀=174, C₀=7566, K₁≈0.859 — эти значения зависят от параметров сценария и не захардкожены.

Для чистовой гл. 3: вставить **фактический вывод** `LabOptimiser::formatReport` для `demo_two_specimens`.

---

## Входы и выходы программы

### Вход (`ScenarioBundle`)

```text
ScenarioBundle
├── name, description
├── programMode          # BasicMechanical | MechanicalExtended | Thermomechanical
├── problem: ProblemDefinition
│   ├── specimens[]      # S
│   ├── operations[]     # D
│   ├── equipment[]      # E
│   ├── laboratory       # G, Z
│   ├── required, capable, precedence
│   ├── laborRatePerHour, gridCellSizeM, minutesPerGridStep
│   └── objectiveMode, weights
```

Загрузка: встроенные демо (`buildDemo*`), частичный JSON (`scenario_json`), сборка из режима (`buildFromProgram`).

### Выход (`PipelineOutput`)

| Артефакт | Содержание |
|----------|------------|
| Консоль / GUI | Один оптимальный план: T_sum, T_cycle, C, N, L, η, разложение C |
| `output/report_<name>.txt` | Текстовый отчёт + карта размещения |
| `output/plan.csv` | Агрегированные метрики и статьи C |
| `output/<name>_saved.json` | Сохранение входных данных (GUI) |

---

## §3.7 — что добавить в чистовую главу 3

Краткий текст для диплома (источник — [`architecture.md`](architecture.md)):

1. **Препроцессор** — загрузка сценария, `validate()` (партия, Cap, матрица состояний, предшествование).
2. **Ядро** — `RoutePlanner` (два маршрута), `RouteAnalyzer`, `StandStateAnalyzer`, `MetricsEngine`, `LabOptimiser`.
3. **Постпроцессор** — отчёт TXT, CSV.
4. **GUI** — четыре режима программы, расчёт, экспорт.
5. **IO** — JSON входных данных; полный парсер — в разработке.

Скриншоты для диплома: экран выбора режима, фрагмент `formatReport`, `output/plan.csv`.

---

## Терминология (не путать)

| В тексте | В коде | Не использовать |
|----------|--------|-----------------|
| Партия S | `specimens` | «множество образцов» |
| Операции D | `operations` | — |
| Матрица состояний | `StandStateMatrix` | «матрица переходов» |
| Целевая функция | `VariantMetrics::K` или C | «цель оптимизации» = K |
| Ячейка 2×2 м | `gridCellSizeM` | расчёт по м² |

НК и твердомер **не входят** в режимы программы (брифинг 28.05).
