#pragma once

#include "productionStep.hpp"
#include "../part.hpp"

//
class ProductionSemiproduct {
 private:
    sf::RectangleShape background;
    GUI::DynamicText diameterText;
    GUI::DynamicText massText;
    GUI::DynamicText lengthText;
    GUI::DynamicText rougnessText;

 public:
    ProductionSemiproduct(Window& window, float X, float Y);
    void draw(Window& window);
    void setNewParameters(Part part);
};
