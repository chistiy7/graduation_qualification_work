#include "part.hpp"


Material Part::material(Material::MachineSteel);


Part::Part(float _diameter, float _length, float _rougness)
: diameter(_diameter),
length(_length),
rougness(_rougness) {}

void Part::set(float _diameter, float _length, float _rougness) {
    diameter = _diameter;
    length = _length;
    rougness = _rougness;
    updateMass();
}

void Part::set(const Part part) {
    set(part.diameter, part.length, part.rougness);
}

void Part::setMaterial(Material _material) {
    material = _material;
}

void Part::updateMass() {
    mass = diameter*diameter*M_PI/4*length/1000000000*material.density();
}

void Part::saveToFile(std::ofstream& fout) {
    switch (LanguagedText::getLanguage()) {
    case Language::English:
        fout << "d = " << diameter << " mm, l = " << length << " mm, Rz = " << rougness << " mkm;\n";
        break;

    case Language::Russian:
        fout << "d = " << diameter << " мм, l = " << length << " мм, Rz = " << rougness << " мкм;\n";
    }
}
