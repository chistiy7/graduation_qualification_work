#include "GUI.hpp"


GUI::WarningBox::WarningBox(Window& window, float X, float Y, LanguagedText _texts, const std::string _imageFile)
: text(window.font, _texts.getUTF8()),
texture(resourcesDir() / _imageFile),
sprite(texture) {
    text.setPosition({X, Y});
    sprite.setPosition({X-5-sprite.getGlobalBounds().size.x, Y});
}

void GUI::WarningBox::draw(Window& window) {
    if (active) {
        window.draw(sprite);
        window.draw(text);
    }
}

void GUI::WarningBox::activate() {
    active = true;
}

void GUI::WarningBox::deactivate() {
    active = false;
}
