#include <cmath>
#include <fstream>
#include "../testing.hpp"
#include "mechanicalStage.hpp"


MechanicalStage::MechanicalStage() {
    setCapacity(1);
}

void MechanicalStage::set(Part outPart, unsigned _stepNumber, float _settedPartCapacity, int _batchVolume) {
    setFirst(outPart, _stepNumber, getRougness(_stepNumber), _settedPartCapacity, _batchVolume);
}

void MechanicalStage::setFirst(Part outPart, unsigned _stepNumber, float _inputRougness, float _settedPartCapacity, int _batchVolume) {
    inputPart = {calculateInputDiameter(_stepNumber, outPart.diameter), calculateInputLength(_stepNumber, outPart.length), _inputRougness};
    activeTime = calculateTime();
    setOperationTime(activeTime + (calcualteMountTime(inputPart)+calcualteMountTime(outPart))/60 + calculateInstrumentTime(activeTime), _batchVolume);
    setRequiredQuantity(_settedPartCapacity, _batchVolume);
    setPowerConsumption(powerInput);
    setCost(_batchVolume, activeTime, calculateInstrumentCost(activeTime));

    // Check correction of calculated numbers
    #if CHECK_CORRECTION
    std::ofstream fout("Calculations.txt");
    fout << "d=" << inputPart.diameter << " l=" << inputPart.length << " Rz=" << inputPart.rougness << '\n';
    fout << "Подача: " << calculateToolFeed() << '\n';
    fout << "Скорость резания: " << calculateCutSpeed() << '\n';
    fout << "Частота вращения: " << calculateRotateFrequency() << '\n';
    fout << "Минутная подача: " << calculateMinuteFeed() << '\n';
    fout << "Время: " << calculateTime() << '\n';
    #endif
}

int MechanicalStage::getStepNumber(float rougness) {
    if (rougness <= 6.4) {
        return 3;
    } else if (rougness <= 12.8) {
        return 2;
    } else if (rougness <= 50) {
        return 1;
    }
    return 0;
}

float MechanicalStage::getRougness(int stepIndex) {
    switch (stepIndex) {
    case 0:
        return 200;

    case 1:
        return 50;
    
    case 2:
        return 12.8;

    case 3:
        return 6.4;
    }
    return 200;
}

float MechanicalStage::calculateCutDistance(unsigned step, float diameter) {
    switch (step) {
    case 0:
        if (diameter < 18) {
            return 2.0;
        } else if (diameter < 30) {
            return 2.5;
        } else if (diameter < 50) {
            return 3.0;
        } else if (diameter < 80) {
            return 3.5;
        } else if (diameter < 120) {
            return 4.5;
        } else if (diameter < 180) {
            return 5.0;
        } else if (diameter < 250) {
            return 5.5;
        } else if (diameter < 320) {
            return 6.5;
        } else if (diameter < 400) {
            return 7.5;
        }
        return 8.5;
    
    case 1:
        if (diameter < 18) {
            return 0.9;
        } else if (diameter < 30) {
            return 1.0;
        } else if (diameter < 50) {
            return 1.3;
        } else if (diameter < 80) {
            return 1.5;
        } else if (diameter < 120) {
            return 1.7;
        } else if (diameter < 180) {
            return 2.0;
        } else if (diameter < 250) {
            return 2.2;
        } else if (diameter < 320) {
            return 2.4;
        } else if (diameter < 400) {
            return 2.6;
        }
        return 2.8;

    case 2:
        if (diameter < 18) {
            return 0.5;
        } else if (diameter < 30) {
            return 0.6;
        } else if (diameter < 50) {
            return 0.7;
        } else if (diameter < 80) {
            return 0.8;
        } else if (diameter < 120) {
            return 0.9;
        } else if (diameter < 180) {
            return 1.0;
        } else if (diameter < 250) {
            return 1.1;
        } else if (diameter < 320) {
            return 1.2;
        } else if (diameter < 400) {
            return 1.4;
        }
        return 1.6;

    case 3:
        if (diameter < 30) {
            return 0.2;
        } else if (diameter < 120) {
            return 0.3;
        } else if (diameter < 400) {
            return 0.4;
        }
        return 0.6;
    }
    return 0;
}

float MechanicalStage::calculateInputDiameter(unsigned step, float outDiameter) {
    return outDiameter+2*calculateCutDistance(step, outDiameter);
}

float MechanicalStage::calculateInputLength(unsigned step, float outLength) {
    return outLength;
}

float MechanicalStage::calculateToolFeed() {
    return (inputPart.rougness/1000)*sinf(Part::material.mainFi()+Part::material.addFi())/sinf(Part::material.mainFi())/sinf(Part::material.addFi());
}

float MechanicalStage::calculateCutSpeed() {
    return Part::material.Cv(calculateToolFeed()) / powf(normTimeCut, 0.4);
}

float MechanicalStage::calculateRotateFrequency() {
    return std::roundf(1000*calculateCutSpeed()/inputPart.diameter/M_PI);
}

float MechanicalStage::calculateMinuteFeed() {
    return calculateToolFeed()*calculateRotateFrequency();
}

float MechanicalStage::calculateTime() {
    return (inputPart.length+2)/(calculateMinuteFeed());
}

float MechanicalStage::calcualteMountTime(Part part) {
    // Manual installation
    if (part.mass < 0.25) {
        return 0.16;
    } else if (part.mass < 0.5) {
        return 0.17;
    } else if (part.mass < 1) {
        if (part.diameter > 250) {
            return 0.18;
        } else {
            return 0.25;
        }
    } else if (part.mass < 3) {
        if (part.diameter > 250) {
            return 0.19;
        } else {
            return 0.29;
        }
    } else if (part.mass < 5) {
        if (part.diameter > 250) {
            return 0.22;
        } else {
            return 0.34;
        }
    } else if (part.mass < 8) {
        if (part.diameter > 250) {
            return 0.26;
        } else {
            return 0.38;
        }
    } else if (part.mass < 12) {
        if (part.diameter > 250) {
            return 0.32;
        } else {
            return 0.46;
        }
    } else if (part.mass < 20) {
        if (part.diameter > 250) {
            return 0.39;
        } else {
            return 0.6;
        }
    }
    // Mechanical installation
    return 0.6+1.5;
}

float MechanicalStage::calculateInstrumentTime(float activeTime) {
    return ceilf(activeTime/normTimeCut) * 1.5/60;
}

float MechanicalStage::calculateInstrumentCost(float activeTime) {
    return ceilf(activeTime/normTimeCut) * instrumentCost;
}

// Getting public functions
Part MechanicalStage::getInputPart() {
    return inputPart;
}

void MechanicalStage::saveToFile(std::ofstream& fout) {
    switch (LanguagedText::getLanguage()) {
    case Language::English:
        fout << "Mechanical stage:\n";
        // Process info
        fout << "Tool feed: " << calculateToolFeed() << " mm; ";
        fout << "Cutting speed: " << calculateCutSpeed() << " mm/min;\n";
        fout << "Rotation frequency: " << calculateRotateFrequency() << " rpm; ";
        fout << "Minute feed: " << calculateMinuteFeed() << " mm/min;\n";
        fout << "Active time: " << activeTime << " hours;\n";
        // Other data
        fout << "Time per operation: " << getTimePerOperation() << " hours;\n";
        fout << "Required: " << getReqieredQuantity() << " machines;\n";
        fout << "Cost per part: " << getPartCost() << " rub;\n";
        break;

    case Language::Russian:
        fout << "Механическая обработка:\n";
        // Process info
        fout << "Подача: " << calculateToolFeed() << " мм; ";
        fout << "Скорость резания: " << calculateCutSpeed() << " мм/мин;\n";
        fout << "Частота вращения: " << calculateRotateFrequency() << " оборотов/мин; ";
        fout << "Минутная подача: " << calculateMinuteFeed() << " мм/мин;\n";
        fout << "Время резания: " << activeTime << " часов;\n";
        // Other data
        fout << "Длительность производства детали: " << getTimePerOperation() << " часов;\n";
        fout << "Необходимо: " << getReqieredQuantity() << " станков;\n";
        fout << "Стоимость одной детали: " << getPartCost() << " рублей;\n";
    }
}
