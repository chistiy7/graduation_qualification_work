#pragma once

#include "model/problem.hpp"

namespace lab {

struct LayoutAreaStats {
    int equipmentCells = 0;
    int reservedCells = 0;
    int freeCells = 0;
    double usedAreaM2 = 0.0;
};

[[nodiscard]] LayoutAreaStats computeLayoutAreaStats(const ProblemDefinition& problem);

[[nodiscard]] double areaOccupancyCost(const ProblemDefinition& problem, double cycleTimeMin);

}  // namespace lab
