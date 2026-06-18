#include "model/scenario_validation.hpp"

#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace lab {

namespace {

std::unordered_set<std::string> routableOperationIds(const ProblemDefinition& problem) {
    std::unordered_set<std::string> ids;
    for (const auto& op : problem.operations) {
        ids.insert(op.id);
    }
    return ids;
}

std::string formatSpecimenReuseError(const std::string& specimenId) {
    return "Ошибка сценария: образец " + specimenId +
           " назначен на несколько отдельных испытаний. "
           "В текущей модели один физический образец может проходить только одно испытание, "
           "так как испытание изменяет или разрушает образец. "
           "Для комбинированной нагрузки используйте отдельный тип операции, "
           "например tension_torsion.";
}

}  // namespace

std::vector<std::string> validateSpecimenOneTestRule(const ProblemDefinition& problem) {
    const auto routable = routableOperationIds(problem);
    std::vector<std::string> errors;

    for (const auto& specimen : problem.specimens) {
        const auto sit = problem.required.find(specimen.id);
        if (sit == problem.required.end()) continue;

        std::vector<std::string> assigned;
        for (const auto& [opId, required] : sit->second) {
            if (required && routable.count(opId)) {
                assigned.push_back(opId);
            }
        }

        if (assigned.size() > 1) {
            std::ostringstream detail;
            detail << formatSpecimenReuseError(specimen.id);
            detail << " (назначено: ";
            for (size_t i = 0; i < assigned.size(); ++i) {
                if (i > 0) detail << ", ";
                detail << assigned[i];
            }
            detail << ")";
            errors.push_back(detail.str());
        }
    }

    return errors;
}

std::vector<std::string> validateRouteOneSpecimenOneStep(const TestRoute& route) {
    std::vector<std::string> errors;
    std::unordered_map<std::string, std::string> operationBySpecimen;

    for (const auto& step : route.steps()) {
        const auto it = operationBySpecimen.find(step.specimenId);
        if (it == operationBySpecimen.end()) {
            operationBySpecimen.emplace(step.specimenId, step.operationId);
            continue;
        }
        if (it->second != step.operationId) {
            errors.push_back(formatSpecimenReuseError(step.specimenId) + " (маршрут: " +
                             it->second + " и " + step.operationId + ")");
        } else {
            errors.push_back("Ошибка маршрута: образец " + step.specimenId +
                             " встречается в маршруте более одного раза.");
        }
    }

    return errors;
}

bool specimenAppearsOnceInRoute(const TestRoute& route) {
    std::unordered_set<std::string> seen;
    for (const auto& step : route.steps()) {
        if (!seen.insert(step.specimenId).second) return false;
    }
    return true;
}

}  // namespace lab
