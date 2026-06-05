#pragma once

#include <string>
#include <vector>

namespace lab {

// Одна запланированная операция в маршруте
struct RouteStep {
    std::string specimenId;
    std::string operationId;
    std::string equipmentId;
};

// Маршрут испытаний партии
class TestRoute {
public:
    void addStep(RouteStep step) { steps_.push_back(std::move(step)); }
    [[nodiscard]] const std::vector<RouteStep>& steps() const { return steps_; }

private:
    std::vector<RouteStep> steps_;
};

}  // namespace lab
