# Утверждённый перечень испытательных операций и режимов

Источник: гл. 1 (классификация, комбинированные режимы), гл. 2 (стенды, зоны, ограничения).

Код: `src/domain/test_operation_catalog.*`, `stand_catalog.*`, `model/lab_program_mode.*`.

## Виды испытательных операций

| ID | Операция | Режим нагружения |
|----|----------|------------------|
| prep | Подготовка образца | Auxiliary |
| geom | Контроль геометрии | Auxiliary |
| tension | Растяжение | ShortTerm |
| compression | Сжатие | ShortTerm |
| torsion | Кручение | ShortTerm |
| bending | Изгиб | ShortTerm |
| fatigue | Усталость | CyclicLong |
| hardness | Твердость (не в режимах программы, НК исключён) | ShortTerm |
| thermal_static | Статический нагрев | Thermal |
| thermal_cyclic | Циклический термический режим | Thermal |
| induction | Индукционный нагрев | Thermal |
| thermo_mech | Термомеханическое испытание | Thermomechanical |
| cooling | Охлаждение | Auxiliary |
| results | Обработка результатов | Auxiliary |
| tension_torsion | Растяжение + кручение | CombinedSequential |
| bending_torsion | Изгиб + кручение | CombinedSequential |
| compression_bending | Сжатие + изгиб | CombinedSequential |

## Испытательные стенды (гл. 2, §2.1)

| Тип | Стенд | Операции |
|-----|-------|----------|
| UniversalTensile | Разрывная машина | tension, compression, tension_torsion |
| TorsionMachine | Машина кручения | torsion, комбинированные с кручением |
| BendingRig | Стенд изгиба | bending, комбинированные с изгибом |
| FatigueStand | Усталость | fatigue |
| HardnessTester | Твердомер | hardness |
| ThermalFurnace | Печь | thermal_static, thermal_cyclic |
| InductionHeater | Индукция | induction |
| ThermomechanicalUnit | Термомеханика | thermo_mech |

## Режимы программы лаборатории (`LabProgramMode`)

| Режим | Содержание |
|-------|------------|
| **BasicMechanical** | растяжение, кручение (демо) |
| **MechanicalExtended** | + изгиб, усталость |
| **ThermalCycle** | термический нагрев, индукция, охлаждение |
| **Thermomechanical** | термомеханика и комбинированные режимы |

Сценарий собирается через `buildFromProgram(ProgramBuildRequest)` — параметры времени/стоимости задаются в запросе, не в коде метрик.
