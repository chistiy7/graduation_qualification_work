#pragma once

#include "../global/submenuCycle.hpp"
#include "factory.hpp"

// Subwindow for create and set parameters of factory grid
class FactoryCycle : public SubmenuCycle {
private:
    // Main object, representing whole system
    Factory factory;

    // GUI to interact with factory grid
    GUI::Text widthText, heightText;
    GUI::TypeBox widthBox, heightBox;
    GUI::SwitchBox<5> cellTypeSwitch;
    GUI::TextButton optimizeButton;
    GUI::TextButton updatePathButton;
    GUI::Text pathTypeText;

    // Current selected object to place
    bool selectObject = false;
    Cell cursorCell;

public:
    FactoryCycle(Window& window);
    void keyDown(sf::Event::KeyPressed state) override;
    void LClick(sf::Vector2i pos) override;
    void LUnClick(sf::Vector2i pos) override;
    void RClick(sf::Vector2i pos) override;
    void textInput(char32_t keyCode) override;
    void update() override;
    void draw() override;
};
