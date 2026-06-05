#include "productionSemiproduct.hpp"

ProductionSemiproduct::ProductionSemiproduct(Window& window, float X, float Y)
: background({160, 160}),
diameterText(window, X+2, Y+5, {"d={:.1f} mm", "d={:.1f} мм"}, GUI::Aligment::Left),
lengthText(window, X+2, Y+40, {"l={:.0f} mm", "l={:.0f} мм"}, GUI::Aligment::Left),
massText(window, X+2, Y+75, {"m={:.2f} kg", "m={:.2f} кг"}, GUI::Aligment::Left),
rougnessText(window, X+2, Y+110, {"Rz={:.1f}", "Rz={:.1f}"}, GUI::Aligment::Left) {
    background.setPosition({X, Y});
    background.setFillColor(sf::Color{120, 120, 120});
    background.setOutlineThickness(2);
    background.setOutlineColor(sf::Color::Black);
}

void ProductionSemiproduct::draw(Window& window) {
    window.draw(background);
    diameterText.draw(window);
    lengthText.draw(window);
    massText.draw(window);
    rougnessText.draw(window);
}

void ProductionSemiproduct::setNewParameters(Part part) {
    diameterText.setValues(part.diameter);
    lengthText.setValues(part.length);
    massText.setValues(part.mass);
    rougnessText.setValues(part.rougness);
}
