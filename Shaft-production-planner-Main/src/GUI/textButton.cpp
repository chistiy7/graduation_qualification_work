#include "GUI.hpp"
#include <iostream>


GUI::TextButton::TextButton(Window& win, float X, float Y, float W, float H, LanguagedText _texts, Aligment _aligment)
: rect({W, H}),
text(win.font, _texts.getUTF8()) {
    // Moving origin depend on aligment
    switch (_aligment) {
    case Aligment::Midle:
        rect.setOrigin({rect.getGlobalBounds().size.x/2, 0});
        text.setOrigin({text.getGlobalBounds().size.x/2, 0});
        break;

    case Aligment::Right:
        rect.setOrigin({rect.getGlobalBounds().size.x, 0});
        text.setOrigin({text.getGlobalBounds().size.x, 0});
        break;
    }
    // Creating and placing back rect
    rect.setPosition({X, Y});
    rect.setFillColor(sf::Color{120, 120, 120});
    rect.setOutlineColor(sf::Color::Black);
    rect.setOutlineThickness(2);

    // Creating and placing text
    text.setPosition({X, Y});
    text.setFillColor(sf::Color::White);
}

GUI::TextButton::TextButton(Window& window, float X, float Y, LanguagedText _texts, Aligment _aligment)
: text(window.font, _texts.getUTF8()) {
    // Setting rect width and height depend on text
    rect.setSize({text.getGlobalBounds().size.x+5, text.getGlobalBounds().size.y+15});

    // Moving origin depend on aligment
    switch (_aligment) {
    case Aligment::Midle:
        rect.setOrigin({rect.getGlobalBounds().size.x/2, 0});
        text.setOrigin({text.getGlobalBounds().size.x/2, 0});
        break;

    case Aligment::Right:
        rect.setOrigin({rect.getGlobalBounds().size.x, 0});
        text.setOrigin({text.getGlobalBounds().size.x, 0});
        break;
    }
    // Creating and placing back rect
    rect.setPosition({X, Y});
    rect.setFillColor(sf::Color{120, 120, 120});
    rect.setOutlineColor(sf::Color::Black);
    rect.setOutlineThickness(2);

    // Creating and placing text
    text.setPosition({X, Y});
    text.setFillColor(sf::Color::White);
}

bool GUI::TextButton::isClicked(sf::Vector2i point) {
    return rect.getGlobalBounds().contains({(float)point.x, (float)point.y});
}

void GUI::TextButton::draw(Window& window) {
    window.draw(rect);
    window.draw(text);
}
