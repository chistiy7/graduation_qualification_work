#pragma once

#include <cmath>
#include "../app/languages.hpp"


// Class represemting material of creating part
class Material {
public:
    // Allowable materials
    enum Value{
        MachineSteel,
        Alloys,
        HeatResistantSteel,
    };
    Material() = default;
    Material(unsigned index);
    constexpr Material(Value _value) : value(_value) {}
    constexpr operator Value() const { return value; }
    operator unsigned();

    // Properties of material
    float recristalisationTemp();
    float density();  // Density in kg/m^3
    float mainFi();   // Main cutting fi in deg
    float addFi();    // Additional cutting fi in deg
    float Cv(float toolFeed);  // 
    const std::string getName();

private:
    Value value;
};
