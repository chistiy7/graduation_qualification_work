#include "GUI.hpp"


template <unsigned count>
GUI::SwitchBox<count>::SwitchBox(Window& window, float X, float Y, LanguagedText _texts[count], unsigned startOption)
: arrowTexture(resourcesDir() / "GUI/switchArrow.png"),
arrowSprite(arrowTexture) {
    // Placing arrow sprite
    arrowSprite.setPosition({X, Y});

    // Placing select options
    float maxWidth = 0;
    for (int i=0; i < count; ++i) {
        drawnTexts.emplace_back(window.font, _texts[i].getUTF8());
        drawnTexts[i].setPosition({X+30, Y+i*30-4});
        drawnTexts[i].setFillColor(sf::Color::Black);
        maxWidth = std::max(maxWidth, drawnTexts[i].getGlobalBounds().size.x);
    }
    // Updating start option
    selected = startOption;
    drawnTexts[selected].move({0, -(float)selected*30.0f});

    // Setting background of active text
    backgroundBox.setSize({maxWidth+35, 30});
    backgroundBox.setPosition({X, Y});
}

template <unsigned count>
bool GUI::SwitchBox<count>::click(sf::Vector2i point) {
    if (opened) {
        if ((backgroundBox.getGlobalBounds().position.x < point.x)
        && (backgroundBox.getGlobalBounds().position.y < point.y)
        && (backgroundBox.getGlobalBounds().position.x + backgroundBox.getSize().x > point.x)
        && (backgroundBox.getGlobalBounds().position.y + backgroundBox.getSize().y > point.y)) {
            // Finding new option
            selected = (point.y - backgroundBox.getGlobalBounds().position.y) / 30;
            drawnTexts[selected].move({0, -(float)selected*30.0f});
            opened = false;
            backgroundBox.setSize({backgroundBox.getSize().x, 30});
            return true;
        } else {
            opened = false;
            backgroundBox.setSize({backgroundBox.getSize().x, 30});
            drawnTexts[selected].move({0, -(float)selected*30.0f});
        }
    } else {
        if ((backgroundBox.getGlobalBounds().position.x < point.x)
        && (backgroundBox.getGlobalBounds().position.y < point.y)
        && (backgroundBox.getGlobalBounds().position.x + backgroundBox.getSize().x > point.x)
        && (backgroundBox.getGlobalBounds().position.y + backgroundBox.getSize().y > point.y)) {
            opened = true;
            backgroundBox.setSize({backgroundBox.getSize().x, count*30});
            // Resetting selected postion
            drawnTexts[selected].move({0, (float)selected*30.0f});
        }
    }
    return false;
}

template <unsigned count>
void GUI::SwitchBox<count>::draw(Window& window) {
    // Draw open or close version depend on state
    if (opened) {
        window.draw(backgroundBox);
        for (int i=0; i < count; ++i) {
            window.draw(drawnTexts[i]);
        }
        window.draw(arrowSprite);
    } else {
        window.draw(backgroundBox);
        window.draw(drawnTexts[selected]);
        window.draw(arrowSprite);
    }
}

template <unsigned count>
void GUI::SwitchBox<count>::set(unsigned value) {
    if (opened) {
        selected = value;
        opened = false;
        backgroundBox.setSize({backgroundBox.getSize().x, 30});
        drawnTexts[selected].move({0, -(float)selected*30.0f});
    } else {
        // Resetting old option
        drawnTexts[selected].move({0, (float)selected*30.0f});

        // Moving new option
        selected = value;
        drawnTexts[selected].move({0, (float)selected*-30.0f});
    }
}

template <unsigned count>
unsigned GUI::SwitchBox<count>::getValue() {
    return selected;
}
