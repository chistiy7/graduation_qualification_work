#pragma once

#include <fstream>
#include <string>
#include "../process.hpp"
#include "productionStep.hpp"
#include "productionSemiproduct.hpp"

//
class ProductProcess : Process {
 private:
    // Drawable objects
    ProductionStep drawSteps[4];
    ProductionSemiproduct drawSemiproducts[5];
    GUI::WarningBox warningLowLength, warningLowDiameter;  // Red warnings - unavaliable to produce
    GUI::WarningBox warningHighLength, warningHighDiameter;  // Yellow warnings - ineffecient to produce
    GUI::WarningBox warningHighBlankRoughness;  // Yellow warnings - Too low start roughness - ineffient

    // Function for update visual part after changes
    void update();

 public:
    ProductProcess(Window& window);
    void setMaterial(unsigned index);
    void setTargetRoughness(float roughness);
    void setTargetLength(float length);
    void setTargetDiameter(float diameter);
    void setBlankRoughness(float roughness);
    void setBlankLength(float length);
    void setBlankDiameter(float diameter);
    std::string getTargetRoughness();
    std::string getTargetLength();
    std::string getTargetDiameter();
    std::string getBlankRoughness();
    std::string getBlankLength();
    std::string getBlankDiameter();
    unsigned getMaterial();
    void draw(Window& window);
    static void save(std::ofstream& fout);
    static void load(std::ifstream& fin);
    static void saveInput(std::ofstream& fout);
    static void saveOutput(std::ofstream& fout);
};
