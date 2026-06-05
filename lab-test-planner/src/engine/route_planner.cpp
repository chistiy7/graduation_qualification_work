#include "engine/route_planner.hpp"

#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace lab {

namespace {

bool isRequired(const ProblemDefinition& p, const std::string& specimenId,
                const std::string& operationId) {
    auto sit = p.required.find(specimenId);
    if (sit == p.required.end()) return false;
    auto oit = sit->second.find(operationId);
    return oit != sit->second.end() && oit->second;
}

std::string assignEquipment(const ProblemDefinition& p, const std::string& operationId) {
    for (const auto& e : p.equipment) {
        auto eit = p.capable.find(e.id);
        if (eit == p.capable.end()) continue;
        auto oit = eit->second.find(operationId);
        if (oit != eit->second.end() && oit->second) {
            return e.id;
        }
    }
    throw std::runtime_error("no equipment for operation: " + operationId);
}

std::vector<const TestStage*> requiredOpsForSpecimen(const ProblemDefinition& problem,
                                                     const std::string& specimenId) {
    std::vector<const TestStage*> ops;
    for (const auto& op : problem.operations) {
        if (isRequired(problem, specimenId, op.id)) {
            ops.push_back(&op);
        }
    }
    return ops;
}

// Упорядочивание с учётом Pred (§2.3)
std::vector<const TestStage*> sortByPrecedence(
    const ProblemDefinition& problem, const std::vector<const TestStage*>& ops) {
    std::unordered_map<std::string, int> indexById;
    for (size_t i = 0; i < ops.size(); ++i) {
        indexById[ops[i]->id] = static_cast<int>(i);
    }

    auto precedes = [&](const std::string& a, const std::string& b) {
        for (const auto& [pred, succ] : problem.precedence) {
            if (pred == a && succ == b) return true;
        }
        return false;
    };

    std::vector<const TestStage*> sorted = ops;
    std::sort(sorted.begin(), sorted.end(), [&](const TestStage* x, const TestStage* y) {
        if (precedes(x->id, y->id)) return true;
        if (precedes(y->id, x->id)) return false;
        return indexById[x->id] < indexById[y->id];
    });
    return sorted;
}

void appendSpecimenSteps(TestRoute& route, const ProblemDefinition& problem,
                         const std::string& specimenId,
                         const std::vector<const TestStage*>& opOrder) {
    for (const auto* op : opOrder) {
        route.addStep({specimenId, op->id, assignEquipment(problem, op->id)});
    }
}

}  // namespace

bool RoutePlanner::isRequired(const ProblemDefinition& p, const std::string& specimenId,
                              const std::string& operationId) const {
    return ::lab::isRequired(p, specimenId, operationId);
}

std::string RoutePlanner::assignEquipment(const ProblemDefinition& p,
                                          const std::string& operationId) const {
    return ::lab::assignEquipment(p, operationId);
}

TestRoute RoutePlanner::buildBaselineRoute(const ProblemDefinition& problem) const {
    TestRoute route;
    for (const auto& specimen : problem.specimens) {
        auto ops = requiredOpsForSpecimen(problem, specimen.id);
        ops = sortByPrecedence(problem, ops);
        appendSpecimenSteps(route, problem, specimen.id, ops);
    }
    return route;
}

TestRoute RoutePlanner::buildGroupedRoute(const ProblemDefinition& problem) const {
    TestRoute route;
    // Группировка по типу операции (снижает N перенастроек)
    std::unordered_set<std::string> seenOps;
    std::vector<const TestStage*> opSequence;
    for (const auto& op : problem.operations) {
        opSequence.push_back(&op);
    }

    for (const auto* op : opSequence) {
        bool anyRequired = false;
        for (const auto& specimen : problem.specimens) {
            if (isRequired(problem, specimen.id, op->id)) {
                anyRequired = true;
                break;
            }
        }
        if (!anyRequired) continue;

        for (const auto& specimen : problem.specimens) {
            if (!isRequired(problem, specimen.id, op->id)) continue;
            route.addStep({specimen.id, op->id, assignEquipment(problem, op->id)});
        }
        seenOps.insert(op->id);
    }
    return route;
}

}  // namespace lab
