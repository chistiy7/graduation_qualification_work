#pragma once

#include "domain/test_route.hpp"
#include "model/problem.hpp"

#include <string>
#include <vector>

namespace lab {

// Состояния стенда (матрица состояний, брифинг 28.05 — не «матрица переходов»)
enum class StandState {
    Idle,           // свободен
    Busy,           // занят (выполняется операция)
    SetupRequired,  // требуется переналадка
};

struct StandStateEvent {
    std::string equipmentId;
    std::string operationId;
    std::string specimenId;
    StandState stateBefore = StandState::Idle;
    StandState stateAfter = StandState::Busy;
    bool setupPerformed = false;
    bool operationAllowed = true;
};

// Матрица состояний: последовательность состояний стендов по маршруту
struct StandStateMatrix {
    std::vector<StandStateEvent> events;
    int setupCount = 0;
};

class StandStateAnalyzer {
public:
    [[nodiscard]] StandStateMatrix analyze(const ProblemDefinition& problem,
                                           const TestRoute& route) const;

    [[nodiscard]] std::vector<std::string> validatePrecedence(
        const ProblemDefinition& problem, const TestRoute& route) const;
};

}  // namespace lab
