#include <iostream>
#include "GUI.hpp"


GUI::Text::Text(Window& window, float X, float Y, LanguagedText _texts, Aligment _aligment)
: text(window.font, _texts.getUTF8()),
aligment(aligment) {
    switch (_aligment) {
    case Aligment::Midle:
        text.setOrigin({text.getGlobalBounds().size.x/2, 0});
        break;

    case Aligment::Right:
        text.setOrigin({text.getGlobalBounds().size.x, 0});
        break;
    }
    text.setPosition({X, Y});
}

GUI::Text::Text(Window& window, float X, float Y, const std::string _text, Aligment _aligment)
: text(window.font, sf::String::fromUtf8(_text.begin(), _text.end())),
aligment(_aligment) {
    switch (aligment) {
    case Aligment::Midle:
        text.setOrigin({text.getGlobalBounds().size.x/2, 0});
        break;

    case Aligment::Right:
        text.setOrigin({text.getGlobalBounds().size.x, 0});
        break;
    }
    text.setPosition({X, Y});
}

void GUI::Text::setText(LanguagedText texts) {
    text.setString(texts.getUTF8());
    switch (aligment) {
    case Aligment::Midle:
        text.setOrigin({text.getGlobalBounds().size.x/2, 0});
        break;

    case Aligment::Right:
        text.setOrigin({text.getGlobalBounds().size.x, 0});
        break;
    }
}

void GUI::Text::setText(const std::string _text) {
    text.setString(sf::String::fromUtf8(_text.begin(), _text.end()));
    switch (aligment) {
    case Aligment::Midle:
        text.setOrigin({text.getGlobalBounds().size.x/2, 0});
        break;

    case Aligment::Right:
        text.setOrigin({text.getGlobalBounds().size.x, 0});
        break;
    }
}

void GUI::Text::draw(Window& window) {
    window.draw(text);
}
