#pragma once

#include "domain/test_route.hpp"
#include "model/problem.hpp"

namespace lab {

// Формирование обязательного списка испытаний и допустимого оборудования
class RoutePlanner {
public:
    [[nodiscard]] TestRoute buildBaselineRoute(const ProblemDefinition& problem) const;
    [[nodiscard]] TestRoute buildGroupedRoute(const ProblemDefinition& problem) const;

private:
    [[nodiscard]] bool isRequired(const ProblemDefinition& p, const std::string& specimenId,
                                  const std::string& operationId) const;
    [[nodiscard]] std::string assignEquipment(const ProblemDefinition& p,
                                              const std::string& operationId) const;
};

}  // namespace lab
