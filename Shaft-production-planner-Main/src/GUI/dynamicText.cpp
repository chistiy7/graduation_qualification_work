#include "GUI.hpp"


GUI::DynamicText::DynamicText(Window& window, float X, float Y, LanguagedText _texts, Aligment _aligment)
: text(window.font, _texts.getUTF8()),
textTemplate(_texts),
aligment(_aligment) {
	text.setPosition({X, Y});
}

void GUI::DynamicText::updateOrigin() {
    switch (aligment) {
    case Aligment::Midle:
        text.setOrigin({text.getGlobalBounds().size.x/2, 0});
        break;

    case Aligment::Right:
        text.setOrigin({text.getGlobalBounds().size.x, 0});
        break;
    }
}

void GUI::DynamicText::draw(Window& window) {
    window.draw(text);
}
