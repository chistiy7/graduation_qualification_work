#include "ui/window.hpp"

#include <stdexcept>

namespace lab::ui {

AppWindow::AppWindow(unsigned width, unsigned height, const std::string& title)
    : sf::RenderWindow(sf::VideoMode({width, height}), title, sf::Style::Close) {
    const auto fontPath = resourcesDir() / "Arial.ttf";
    if (!font_.openFromFile(fontPath.string())) {
        throw std::runtime_error("font not found: " + fontPath.string());
    }
    setFramerateLimit(60);
}

}  // namespace lab::ui
