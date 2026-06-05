#include "GUI.hpp"


GUI::InfoBox::InfoBox(Window& window, float X, float Y, LanguagedText _texts)
: text(window.font, _texts.getUTF8()) {
    // Setting position
    text.setOrigin({text.getGlobalBounds().size.x/2, 0});
    text.setPosition({X, Y});
}

void GUI::InfoBox::draw(Window& window) {
    // Drawing, until counter reach 0
    if (counter) {
        // Setting text to be less and less visible
        color.a = ((float)counter/100*255);
        text.setFillColor(color);
        window.draw(text);
        counter--;
    }
}

void GUI::InfoBox::reset() {
    counter = 100;
}
