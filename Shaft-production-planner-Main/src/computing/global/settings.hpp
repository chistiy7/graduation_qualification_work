#pragma once

#include "../../app/cycleTemplate.hpp"


class Settings {
private:
    GUI::Text settingText;
    GUI::ImageButton activateButton;
    sf::RectangleShape background;
    GUI::ImageButton languageButtons[2];
    bool selected = false;

public:
    Settings(Window& window);
    void draw(Window& window);
    bool click(sf::Vector2i pos);
};
