#pragma once

#include "domain/test_route.hpp"
#include "model/problem.hpp"

namespace lab {

class MetricsEngine {
public:
    // Вычисляет и возвращает совокупные метрики (время, стоимость, загрузку, узкое место и др.) по входной задаче и маршруту испытаний.
    [[nodiscard]] VariantMetrics compute(const ProblemDefinition& problem,
                                         const TestRoute& route) const;
                                  

private:
    [[nodiscard]] double operationTimeSum(const ProblemDefinition& problem,
                                          const TestRoute& route) const;
    // Возвращает суммарную стоимость только расходных материалов, использованных во всех операциях маршрута (например, материалы, списываемые по техкарте операции).
    [[nodiscard]] double operationMaterialsCost(const ProblemDefinition& problem,
                                              const TestRoute& route) const;
                                       
    // Возвращает суммарную стоимость энергозатрат в рабочем режиме для всего маршрута.
    [[nodiscard]] double energyWorkCost(const ProblemDefinition& problem,
                                        const TestRoute& route) const;
                                
    [[nodiscard]] double prepLaborCost(const ProblemDefinition& problem) const;
    [[nodiscard]] double operationLaborCost(const ProblemDefinition& problem,
                                            const TestRoute& route) const;
    [[nodiscard]] double amortizationCost(const ProblemDefinition& problem) const;
    [[nodiscard]] double averageLoad(const std::vector<EquipmentUtilization>& stats) const;
};

}  // namespace lab
