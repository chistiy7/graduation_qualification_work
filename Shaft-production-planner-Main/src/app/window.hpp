#pragma once

#include <string>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "../testing.hpp"
#include "macroses.hpp"
#include "languages.hpp"


// Class with all
class Window : public sf::RenderWindow {
public:
    const sf::Font font;

public:
    Window(unsigned width, unsigned height, std::string name);
    ~Window();
};
