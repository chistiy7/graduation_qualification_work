#include "material.hpp"


Material::Material(unsigned _index) {
    switch (_index) {
    case 0:
        value = Material::MachineSteel;
        break;
    
    case 1:
        value = Material::Alloys;
        break;

    case 2:
        value = Material::HeatResistantSteel;
        break;
    }
}

Material::operator unsigned() {
    switch (value) {
    case Material::MachineSteel:
        return 0;
    
    case Material::Alloys:
        return 1;

    case Material::HeatResistantSteel:
        return 2;
    }
    return 0;
}


float Material::recristalisationTemp() {
    switch (value) {
    case MachineSteel:
        return 680;

    case Alloys:
        return 450;
    
    case HeatResistantSteel:
        return 850;
    }
    return 0;
}

float Material::density() {
    switch (value) {
    case MachineSteel:
        return 7800;

    case Alloys:
        return 8800;
    
    case HeatResistantSteel:
        return 7700;
    }
    return 0;
}

float Material::mainFi() {
    switch (value) {
    case MachineSteel:
        return 60*M_PI/180;

    case Alloys:
        return 45*M_PI/180;
    
    case HeatResistantSteel:
        return 45*M_PI/180;
    }
    return 0;
}

float Material::addFi() {
    switch (value) {
    case MachineSteel:
        return 15*M_PI/180;

    case Alloys:
        return 10*M_PI/180;
    
    case HeatResistantSteel:
        return 10*M_PI/180;
    }
    return 0;
}

float Material::Cv(float toolFeed) {
    if (toolFeed > 0.4) {
        switch (value) {
        case MachineSteel:
            return 548;

        case Alloys:
            return 436;
        
        case HeatResistantSteel:
            return 345;
        }
    } else {
        switch (value) {
        case MachineSteel:
            return 478;

        case Alloys:
            return 405;
        
        case HeatResistantSteel:
            return 301;
        }
    }
    return 0;
}

const std::string Material::getName() {
    switch (LanguagedText::getLanguage()) {
    case Language::English:
        switch (value) {
        case Material::MachineSteel:
            return "machine steel";
        
        case Material::Alloys:
            return "alloys";

        case Material::HeatResistantSteel:
            return "heat resistant steel";
        }
        return "unknown";

    case Language::Russian:
        switch (value) {
        case Material::MachineSteel:
            return "машиноотделочная сталь";
        
        case Material::Alloys:
            return "цветные сплавы";

        case Material::HeatResistantSteel:
            return "жаропрочные стали";
        }
        return "неизвестно";
    }
    return "unknown";
}
