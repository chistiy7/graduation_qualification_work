#pragma once

#include "app.hpp"
#include "../GUI/GUI.hpp"
#include "../GUI/switchBox.cpp"


// Template class for create and run cycles
class CycleTemplate {
 private:
    void getInput();  // Function for catching all input
    bool running = true;  // Flag of running current cycle

 protected:
    // Target window, where get input and draw to
    Window& window;

    // Function for stopping current cycle
    void stop();

    // Overriding functions for do with input
    virtual void keyDown(sf::Event::KeyPressed state) {}
    virtual void LClick(sf::Vector2i pos) {}
    virtual void RClick(sf::Vector2i pos) {}
    virtual void LUnClick(sf::Vector2i pos) {}
    virtual void RUnClick(sf::Vector2i pos) {}
    virtual void textInput(char32_t keyCode) {}

    // Overring function for another objects update
    virtual void update() {}

    // Overreding function for draw data to screen
    virtual void draw() {}

 public:
    CycleTemplate(Window& window);
    void run();  // Start current cycle
};
