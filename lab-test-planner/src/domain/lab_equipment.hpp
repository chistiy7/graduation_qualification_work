#pragma once

#include "domain/stand_catalog.hpp"

#include <string>

namespace lab {

// Испытательный стенд e ∈ E
struct LabEquipment {
    std::string id;
    std::string cellId;
    StandType standType{};
    std::string nameRu;
    double setupTimeMin = 0.0;       // t_set (мин, НТП)
    double setupCost = 0.0;          // c_set (руб)
    double amortPerHour = 0.0;       // c_am, вкл. расходники/износ
    double fundTimeMin = 0.0;
    double cellPlacementCost = 0.0;  // стоимость размещения одной ячейки стенда (руб)
    DirectionalBuffer buffer;        // направленные зоны безопасности (§2.2, ГОСТ)
    int orientation = 1;             // 0=N 1=E 2=S 3=W — куда обращён «перёд» стенда
    double nominalPowerKw = 10.0;    // P_nom — паспортная мощность, кВт (переналадка)
};

}  // namespace lab
