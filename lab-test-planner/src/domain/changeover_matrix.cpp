#include "domain/changeover_matrix.hpp"

namespace lab {

namespace {

// Индекс операции в матрице переналадки универсального стенда; -1 — вне набора.
int universalIndex(TestOperationType type) {
    switch (type) {
    case TestOperationType::Tension:
        return 0;
    case TestOperationType::Torsion:
        return 1;
    case TestOperationType::TensionPlusTorsion:
        return 2;
    default:
        return -1;
    }
}

// Симметричная матрица времени переналадки, мин.
//            растяж.  кручен.  совмещ.
//  растяж.    12       25       15
//  кручен.    25       12       15
//  совмещ.    15       15       12
constexpr double kChangeoverMin[3][3] = {
    {12.0, 25.0, 15.0},
    {25.0, 12.0, 15.0},
    {15.0, 15.0, 12.0},
};

constexpr double kSpecimenSwapMin = 12.0;  // смена образца как нижняя граница

}  // namespace

bool isUniversalStandOperation(TestOperationType type) {
    return universalIndex(type) >= 0;
}

double changeoverTimeMin(TestOperationType from, TestOperationType to) {
    const int a = universalIndex(from);
    const int b = universalIndex(to);
    if (a < 0 || b < 0) return kSpecimenSwapMin;
    return kChangeoverMin[a][b];
}

}  // namespace lab
