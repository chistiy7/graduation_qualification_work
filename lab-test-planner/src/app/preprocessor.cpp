#include "app/preprocessor.hpp"

#include "engine/route_planner.hpp"
#include "engine/stand_state_matrix.hpp"
#include "io/scenario_json.hpp"
#include "model/scenario_bundle.hpp"
#include "model/scenario_input.hpp"
#include "model/scenario_validation.hpp"

#include <stdexcept>

namespace lab {

ScenarioBundle Preprocessor::loadBuiltin(const std::string& id) const {
    if (id == "simple" || id == "demo_simple") return buildDemoSimple();
    if (id == "two_specimens" || id == "demo_two_specimens" || id == "chapter36") {
        return buildDemoTwoSpecimens();
    }
    if (id == "8_types_80" || id == "demo_8_types_80" || id == "8x10") {
        return buildScenario8Types80();
    }
    throw std::runtime_error("unknown builtin scenario: " + id);
}

ScenarioBundle Preprocessor::loadFromFile(const std::filesystem::path& path) const {
    return loadScenarioJson(path);
}

bool Preprocessor::hasCapableEquipment(const ProblemDefinition& p,
                                       const std::string& operationId) const {
    for (const auto& e : p.equipment) {
        auto eit = p.capable.find(e.id);
        if (eit == p.capable.end()) continue;
        auto oit = eit->second.find(operationId);
        if (oit != eit->second.end() && oit->second) return true;
    }
    return false;
}

std::vector<std::string> Preprocessor::validate(const ScenarioBundle& bundle) const {
    std::vector<std::string> errors;
    const auto& p = bundle.problem;

    if (p.specimens.empty()) errors.push_back("партия образцов пуста");
    if (p.operations.empty()) errors.push_back("нет испытательных операций");
    if (p.equipment.empty()) errors.push_back("нет испытательных стендов");

    for (const auto& s : p.specimens) {
        for (const auto& op : p.operations) {
            auto sit = p.required.find(s.id);
            if (sit == p.required.end()) continue;
            auto rit = sit->second.find(op.id);
            if (rit != sit->second.end() && rit->second && !hasCapableEquipment(p, op.id)) {
                errors.push_back("нет стенда для операции " + op.id + " (образец " + s.id + ")");
            }
        }
    }

    if (p.minutesPerGridStep <= 0) {
        errors.push_back("minutesPerGridStep должен быть > 0");
    }

    if (p.gridCellSizeM <= 0) {
        errors.push_back("gridCellSizeM должен быть > 0 (ячейка 2×2 м)");
    }

    for (const auto& err : validateSpecimenOneTestRule(p)) {
        errors.push_back(err);
    }

    const auto baseline = RoutePlanner{}.buildBaselineRoute(p);
    for (const auto& err : validateRouteOneSpecimenOneStep(baseline)) {
        errors.push_back(err);
    }
    for (const auto& err : validateRouteOneSpecimenOneStep(
             RoutePlanner{}.buildGroupedRoute(p))) {
        errors.push_back(err);
    }

    StandStateAnalyzer stateAnalyzer;
    const auto stateMatrix = stateAnalyzer.analyze(p, baseline);
    for (const auto& ev : stateMatrix.events) {
        if (!ev.operationAllowed) {
            errors.push_back("матрица состояний: стенд " + ev.equipmentId +
                             " не допускает операцию " + ev.operationId);
        }
    }
    for (const auto& err : stateAnalyzer.validatePrecedence(p, baseline)) {
        errors.push_back(err);
    }

    return errors;
}

void Preprocessor::ensureValid(const ScenarioBundle& bundle) const {
    auto errors = validate(bundle);
    if (!errors.empty()) {
        std::string msg = "ошибки входных данных:";
        for (const auto& e : errors) msg += "\n - " + e;
        throw std::runtime_error(msg);
    }
}

}  // namespace lab
