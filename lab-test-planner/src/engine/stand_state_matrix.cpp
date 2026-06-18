#include "engine/stand_state_matrix.hpp"

#include <optional>
#include <unordered_map>

namespace lab {

StandStateMatrix StandStateAnalyzer::analyze(const ProblemDefinition& problem,
                                               const TestRoute& route) const {
    StandStateMatrix matrix;
    std::unordered_map<std::string, std::optional<std::string>> lastModeOnStand;
    std::unordered_map<std::string, StandState> standState;

    for (const auto& step : route.steps()) {
        StandStateEvent ev;
        ev.equipmentId = step.equipmentId;
        ev.operationId = step.operationId;
        ev.specimenId = step.specimenId;
        ev.stateBefore = standState.count(step.equipmentId)
                             ? standState[step.equipmentId]
                             : StandState::Idle;

        bool needsSetup = true;
        if (auto it = lastModeOnStand.find(step.equipmentId);
            it != lastModeOnStand.end() && it->second.has_value()) {
            needsSetup = it->second.value() != step.operationId;
        }

        if (needsSetup) {
            ev.setupPerformed = true;
            ev.stateAfter = StandState::SetupRequired;
            ++matrix.setupCount;
        } else {
            ev.stateAfter = StandState::Busy;
        }

        if (auto capIt = problem.capable.find(step.equipmentId); capIt != problem.capable.end()) {
            auto opIt = capIt->second.find(step.operationId);
            ev.operationAllowed = opIt != capIt->second.end() && opIt->second;
        }

        matrix.events.push_back(ev);
        lastModeOnStand[step.equipmentId] = step.operationId;
        standState[step.equipmentId] = StandState::Busy;
    }

    return matrix;
}

std::vector<std::string> StandStateAnalyzer::validatePrecedence(
    const ProblemDefinition& problem, const TestRoute& route) const {
    std::vector<std::string> errors;
    std::unordered_map<std::string, std::vector<std::string>> doneBySpecimen;

    for (const auto& step : route.steps()) {
        const auto& done = doneBySpecimen[step.specimenId];
        for (const auto& [pred, succ] : problem.precedence) {
            if (succ != step.operationId) continue;

            auto req = problem.required.find(step.specimenId);
            if (req == problem.required.end()) continue;
            auto rit = req->second.find(pred);
            if (rit == req->second.end() || !rit->second) continue;

            bool predDone = false;
            for (const auto& opId : done) {
                if (opId == pred) {
                    predDone = true;
                    break;
                }
            }
            if (!predDone) {
                errors.push_back("нарушение предшествования: " + pred + " должна быть до " +
                                 succ + " (образец " + step.specimenId + ")");
            }
        }
        doneBySpecimen[step.specimenId].push_back(step.operationId);
    }

    return errors;
}

}  // namespace lab
