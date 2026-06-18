#pragma once

#include <string>

namespace lab {

enum class SpecimenRole { Primary, Companion };

// Испытуемый образец s из партии S (не «множество»)
struct Specimen {
    std::string id;
    SpecimenRole role = SpecimenRole::Primary;
    double prepTimeMin = 0.0;
    double prepLaborHours = 0.0;
    // Объём рабочей части образца, м³ (входной параметр; V = π·(d/2)²·l).
    // Используется для расчёта энергии деформации W = σ²/(2E)·V (§3.2).
    double volumeM3 = 3.93e-6;  // по умолчанию: d₀=10 мм, l₀=50 мм (ГОСТ 1497-84, тип IV)
};

}  // namespace lab
