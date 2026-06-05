#include "GUI.hpp"


GUI::ImageButton::ImageButton(float X, float Y, float W, float H, const std::string fileName)
: texture(resourcesDir() / fileName),
sprite(texture) {
    // Placing sprite
    sprite.setScale({W/sprite.getGlobalBounds().size.x, H/sprite.getGlobalBounds().size.y});
    sprite.setOrigin({sprite.getGlobalBounds().size.x/2, sprite.getGlobalBounds().size.y/2});
    sprite.setPosition({X, Y});
}

GUI::ImageButton::ImageButton(float X, float Y, const std::string fileName)
: texture(resourcesDir() / fileName),
sprite(texture) {
    //sprite.setOrigin({sprite.getGlobalBounds().size.x/2, sprite.getGlobalBounds().size.y/2});
    sprite.setPosition({X, Y});
}

bool GUI::ImageButton::isClicked(sf::Vector2i point) {
    return (sprite.getGlobalBounds().position.x < point.x)
        && (sprite.getGlobalBounds().position.y < point.y)
        && (sprite.getGlobalBounds().position.x + sprite.getGlobalBounds().size.x > point.x)
        && (sprite.getGlobalBounds().position.y + sprite.getGlobalBounds().size.y > point.y);
}

void GUI::ImageButton::draw(Window& window) {
    window.draw(sprite);
}
