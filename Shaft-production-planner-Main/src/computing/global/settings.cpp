#include "settings.hpp"


Settings::Settings(Window& window)
: settingText(window, 512, 120, LanguagedText{"Settings", "Настройки"}),
activateButton(1155, 15, "GUI/esc_button.png"),
languageButtons {
    {452, 200, "GUI/Flag_ENG.png"},
    {452, 300, "GUI/Flag_RUS.png"},
} {
    // Setting background part
    background.setFillColor(sf::Color{120, 120, 120});
    background.setSize({200, 400});
    background.setOrigin({100, 200});
    background.setPosition({512, 300});
    background.setOutlineColor(sf::Color::Black);
    background.setOutlineThickness(4);
}

bool Settings::click(sf::Vector2i pos) {
    //
    if (selected) {
        // Checking, if resetting selection
        if ((background.getGlobalBounds().position.x > pos.x)
        || (background.getGlobalBounds().position.y > pos.y)
        || (background.getGlobalBounds().position.x + background.getSize().x < pos.x)
        || (background.getGlobalBounds().position.y + background.getSize().y < pos.y)) {
            selected = false;
            return false;
        } else if (languageButtons[0].isClicked(pos)) {
            selected = false;
            if (LanguagedText::setCurrentLanguage(Language::English)) {
                // Updating all objects
                return true;
            }
        } else if (languageButtons[1].isClicked(pos)) {
            selected = false;
            if (LanguagedText::setCurrentLanguage(Language::Russian)) {
                // Updating all objects
                return true;
            }
        }
    } else {
        if (activateButton.isClicked(pos)) {
            selected = true;
        }
    }
    
    return false;
}

void Settings::draw(Window& window) {
    // Draw activate button
    activateButton.draw(window);

    // Draw changing part
    if (selected) {
        window.draw(background);
        settingText.draw(window);
        languageButtons[0].draw(window);
        languageButtons[1].draw(window);
    }
}
