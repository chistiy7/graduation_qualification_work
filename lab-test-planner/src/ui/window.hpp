#pragma once

#include "app/paths.hpp"

#include <SFML/Graphics.hpp>
#include <string>

namespace lab::ui {

class AppWindow : public sf::RenderWindow {
public:
    AppWindow(unsigned width, unsigned height, const std::string& title);

    [[nodiscard]] const sf::Font& font() const { return font_; }

private:
    sf::Font font_;
};

}  // namespace lab::ui
