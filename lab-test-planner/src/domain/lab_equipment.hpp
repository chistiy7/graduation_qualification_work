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
};

}  // namespace lab
