#include "engine/scheduler.hpp"

#include "domain/changeover_matrix.hpp"
#include "domain/stand_catalog.hpp"
#include "domain/test_stage.hpp"

#include <algorithm>
#include <limits>
#include <optional>
#include <unordered_map>

namespace lab {

namespace {

const LabEquipment* findEquipment(const ProblemDefinition& p, const std::string& id) {
    for (const auto& e : p.equipment) {
        if (e.id == id) return &e;
    }
    return nullptr;
}

const TestStage* findOperation(const ProblemDefinition& p, const std::string& id) {
    for (const auto& o : p.operations) {
        if (o.id == id) return &o;
    }
    return nullptr;
}

double runTimeOf(const TestStage* op) {
    if (!op) return 0.0;
    return op->cycleTimeMin > 0.0 ? op->cycleTimeMin : op->durationMin;
}

double baseSetupTime(const LabEquipment* e, const TestStage* op) {
    if (op && op->programSetupTimeMin > 0.0) return op->programSetupTimeMin;
    return e ? e->setupTimeMin : 0.0;
}

}  // namespace

Schedule Scheduler::build(const ProblemDefinition& problem, const TestRoute& order) const {
    Schedule s;
    const auto& steps = order.steps();
    const size_t n = steps.size();

    // Состояние стендов и оператора.
    std::unordered_map<std::string, double> standFreeAt;  // когда стенд физически свободен
    std::unordered_map<std::string, bool> standOccupied;  // образец на стенде (до снятия)
    std::unordered_map<std::string, std::optional<std::string>> lastOpOnStand;
    std::unordered_map<std::string, std::optional<TestOperationType>> lastTypeOnStand;
    std::unordered_map<std::string, double> standBusy;

    double opFree = 0.0;
    std::string opCell;  // пусто = оператор ещё не у стенда (первый ход без перемещения)

    struct Pending {
        size_t idx = 0;
        double runEnd = 0.0;
        std::string eqId;
        std::string cell;
        std::string specId;
        std::string opId;
    };
    std::vector<Pending> pending;
    size_t next = 0;  // индекс следующей операции к постановке (порядок маршрута)

    auto distTo = [&](const std::string& cell) -> int {
        if (opCell.empty() || cell.empty()) return 0;
        return problem.laboratory.manhattanDistance(opCell, cell);
    };

    auto addPhase = [&](const Pending& p, SchedulePhase phase, double start, double end,
                        bool opBusy) {
        s.phases.push_back({p.specId, p.opId, p.eqId, phase, start, end, opBusy});
        standBusy[p.eqId] += (end - start);
        s.makespanMin = std::max(s.makespanMin, end);
    };

    auto doUnload = [&](size_t pendPos) {
        Pending p = pending[pendPos];
        pending.erase(pending.begin() + static_cast<long>(pendPos));

        const int dist = distTo(p.cell);
        const double travelMin = dist * problem.minutesPerGridStep;
        s.operatorTravelSteps += dist;

        const TestStage* op = findOperation(problem, p.opId);
        const double unloadMin = op ? op->unloadTimeMin : 0.0;

        const double start = std::max(opFree + travelMin, p.runEnd);
        const double end = start + unloadMin;
        if (unloadMin > 0.0) addPhase(p, SchedulePhase::Unload, start, end, true);

        s.operatorBusyMin += travelMin + unloadMin;
        standFreeAt[p.eqId] = end;
        standOccupied[p.eqId] = false;
        opFree = end;
        opCell = p.cell;
    };

    auto startSetup = [&](size_t idx) {
        const auto& step = steps[idx];
        const LabEquipment* e = findEquipment(problem, step.equipmentId);
        const TestStage* op = findOperation(problem, step.operationId);
        const std::string cell = e ? e->cellId : std::string{};

        const int dist = distTo(cell);
        const double travelMin = dist * problem.minutesPerGridStep;
        s.operatorTravelSteps += dist;

        const double avail = opFree + travelMin;
        const double sFree = standFreeAt.count(step.equipmentId) ? standFreeAt[step.equipmentId]
                                                                 : 0.0;
        const double setupStart = std::max(avail, sFree);

        const bool universal = e && e->standType == StandType::UniversalAxialTorsion;
        auto modeIt = lastOpOnStand.find(step.equipmentId);
        const bool firstUse = modeIt == lastOpOnStand.end() || !modeIt->second.has_value();
        const bool programChanged =
            !firstUse && modeIt->second.value() != step.operationId;
        const bool isChangeover = firstUse || programChanged;

        double setupMin = 0.0;
        if (universal) {
            if (firstUse) {
                setupMin = baseSetupTime(e, op);
            } else {
                const TestOperationType prevType = lastTypeOnStand[step.equipmentId].value();
                const TestOperationType curType = op ? op->operationType : prevType;
                setupMin = changeoverTimeMin(prevType, curType);
            }
        } else if (isChangeover) {
            setupMin = baseSetupTime(e, op);
        }
        if (isChangeover) ++s.changeoverCount;

        const double setupEnd = setupStart + setupMin;

        Pending p{idx, 0.0, step.equipmentId, cell, step.specimenId, step.operationId};
        if (setupMin > 0.0) addPhase(p, SchedulePhase::Setup, setupStart, setupEnd, true);

        const double runMin = runTimeOf(op);
        const double runEnd = setupEnd + runMin;
        p.runEnd = runEnd;
        addPhase(p, SchedulePhase::Run, setupEnd, runEnd, false);

        s.operatorBusyMin += travelMin + setupMin;
        opFree = setupEnd;  // оператор свободен после настройки — прогон автономен
        opCell = cell;

        lastOpOnStand[step.equipmentId] = step.operationId;
        if (op) lastTypeOnStand[step.equipmentId] = op->operationType;
        standOccupied[step.equipmentId] = true;
        pending.push_back(p);
    };

    auto earliestFinished = [&](double byTime) -> long {
        long best = -1;
        double bestEnd = std::numeric_limits<double>::max();
        for (size_t k = 0; k < pending.size(); ++k) {
            if (pending[k].runEnd <= byTime && pending[k].runEnd < bestEnd) {
                bestEnd = pending[k].runEnd;
                best = static_cast<long>(k);
            }
        }
        return best;
    };

    auto pendingForStand = [&](const std::string& eqId) -> long {
        for (size_t k = 0; k < pending.size(); ++k) {
            if (pending[k].eqId == eqId) return static_cast<long>(k);
        }
        return -1;
    };

    while (next < n || !pending.empty()) {
        // 1. Снять прогон, завершившийся к моменту освобождения оператора.
        if (long f = earliestFinished(opFree); f >= 0) {
            doUnload(static_cast<size_t>(f));
            continue;
        }

        if (next < n) {
            const auto& step = steps[next];
            // 2a. Нужный стенд занят незавершённым прогоном — дождаться и снять его.
            if (standOccupied.count(step.equipmentId) && standOccupied[step.equipmentId]) {
                const long ps = pendingForStand(step.equipmentId);
                if (ps >= 0) {
                    opFree = std::max(opFree, pending[static_cast<size_t>(ps)].runEnd);
                    doUnload(static_cast<size_t>(ps));
                    continue;
                }
            }

            // 2b. Если до начала настройки придётся ждать — продуктивно снять
            //     прогон, который завершится не позже старта настройки.
            const LabEquipment* e = findEquipment(problem, step.equipmentId);
            const std::string cell = e ? e->cellId : std::string{};
            const double travelMin = distTo(cell) * problem.minutesPerGridStep;
            const double sFree = standFreeAt.count(step.equipmentId)
                                     ? standFreeAt[step.equipmentId]
                                     : 0.0;
            const double setupStart = std::max(opFree + travelMin, sFree);
            if (long f = earliestFinished(setupStart); f >= 0) {
                doUnload(static_cast<size_t>(f));
                continue;
            }

            startSetup(next);
            ++next;
            continue;
        }

        // 3. Операций больше нет — досрочно ждём ближайший прогон и снимаем.
        double soonest = std::numeric_limits<double>::max();
        long pick = -1;
        for (size_t k = 0; k < pending.size(); ++k) {
            if (pending[k].runEnd < soonest) {
                soonest = pending[k].runEnd;
                pick = static_cast<long>(k);
            }
        }
        if (pick < 0) break;
        opFree = std::max(opFree, soonest);
        doUnload(static_cast<size_t>(pick));
    }

    s.operatorIdleMin = std::max(0.0, s.makespanMin - s.operatorBusyMin);
    for (auto& [id, busy] : standBusy) {
        s.standBusyMin.emplace_back(id, busy);
    }
    return s;
}

}  // namespace lab
