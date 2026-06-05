#pragma once

#include "../global/submenuCycle.hpp"
#include "productProcess.hpp"

// Subwindow for create and set properties of part and final product
class ProductCycle : public SubmenuCycle {
private:
    // Main process parameters and solver
    ProductProcess process;

    // Part parameters
    GUI::ImageButton partPicture;
    GUI::TypeBox partWidthInput, partHeightInput, partRoughnessInput;

    // Blank parameters
    GUI::ImageButton blankPicture;
    GUI::TypeBox blankWidthInput, blankHeightInput, blankRoughnessInput;
    GUI::Text materialText;

    // Global part properties
    GUI::SwitchBox<3> materialSwitch;

public:
    ProductCycle(Window& window);
    void LClick(sf::Vector2i pos) override;
    void LUnClick(sf::Vector2i pos) override;
    void RClick(sf::Vector2i pos) override;
    void keyDown(sf::Event::KeyPressed state) override;
    void textInput(char32_t keyCode) override;
    void update() override;
    void draw() override;
};
