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
};

}  // namespace lab
