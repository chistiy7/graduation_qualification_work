#include "engine/layout_area.hpp"

#include "domain/physics_constants.hpp"

namespace lab {

LayoutAreaStats computeLayoutAreaStats(const ProblemDefinition& problem) {
    LayoutAreaStats s;
    for (const auto& c : problem.laboratory.cells()) {
        switch (c.kind) {
        case LabCellKind::Stand:
            ++s.equipmentCells;
            break;
        case LabCellKind::Buffer:
        case LabCellKind::Forbidden:
            ++s.reservedCells;
            break;
        case LabCellKind::Passage:
        case LabCellKind::Cooling:
        case LabCellKind::Staff:
            ++s.freeCells;
            break;
        default:
            break;
        }
    }
    const double cellArea = problem.gridCellSizeM * problem.gridCellSizeM;
    s.usedAreaM2 = static_cast<double>(s.equipmentCells + s.reservedCells) * cellArea;
    return s;
}

double areaOccupancyCost(const ProblemDefinition& problem, double cycleTimeMin) {
    if (cycleTimeMin <= 0.0) return 0.0;
    const auto stats = computeLayoutAreaStats(problem);
    const int usedCells = stats.equipmentCells + stats.reservedCells;
    if (usedCells <= 0) return 0.0;

    const double rent = problem.rentRatePerM2Hour > 0.0 ? problem.rentRatePerM2Hour
                                                        : RENT_RATE_DEFAULT_RUB_M2_HOUR;
    const double cellArea = problem.gridCellSizeM * problem.gridCellSizeM;
    return static_cast<double>(usedCells) * cellArea * rent * (cycleTimeMin / 60.0);
}

}  // namespace lab
