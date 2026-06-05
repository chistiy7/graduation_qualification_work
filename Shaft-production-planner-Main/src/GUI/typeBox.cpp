#include "GUI.hpp"
#if DEBUG
#include <iostream>
#endif


GUI::TypeBox::TypeBox(Window& window, float X, float Y, float W, std::string _startText)
: drawText(window.font, _startText),
backGround({W, 30}),
inverseText(window.font),
inverseRect() {
    // Setting text settings
    drawText.setFillColor(sf::Color::Black);
    drawText.setString(_startText);
    drawText.setPosition({X, Y});
    drawText.setOrigin({-4, drawText.getGlobalBounds().size.y/2-6});
    drawText.setLineSpacing(0.1);

    // Setting background rect
    backGround.setPosition({X, Y});
    backGround.setFillColor(sf::Color::White);
    backGround.setOutlineThickness(2);
    backGround.setOutlineColor(sf::Color::Black);

    // Setting blinking caret
    drawCaret.setFillColor(sf::Color{70});
    drawCaret.setSize({2.f, 30.f});
    drawCaret.setOrigin({1.f, -4.f});

    // Setting inverse selection box
    inverseText.setFillColor(sf::Color::White);
    inverseRect.setOrigin({-4, 0});

    inverseRect.setFillColor(sf::Color::Black);
    inverseRect.setOrigin({0, -6.f});
}

void GUI::TypeBox::clearSelected(const sf::String& str) {
    if (selectLength < 0) {
        drawText.setString(str.substring(0, caret+selectLength) + str.substring(caret));
        caret += selectLength;
    } else {
        drawText.setString(str.substring(0, caret) + str.substring(caret+selectLength));
    }
    selectLength = 0;
}

void GUI::TypeBox::updateInversePos() {
    if (selectLength > 0) {
        inverseRect.setSize({drawText.findCharacterPos(caret+selectLength).x-drawText.findCharacterPos(caret).x, 26.f});
        inverseRect.setPosition({drawCaret.getPosition().x, drawCaret.getPosition().y});

        inverseText.setString(drawText.getString().substring(caret, selectLength));
        inverseText.setPosition(inverseRect.getPosition());
    } else {
        inverseRect.setSize({drawText.findCharacterPos(caret).x-drawText.findCharacterPos(caret+selectLength).x, 26.f});
        inverseRect.setPosition({drawText.findCharacterPos(caret+selectLength).x, drawCaret.getPosition().y});

        inverseText.setString(drawText.getString().substring(caret + selectLength, -selectLength));
        inverseText.setPosition(inverseRect.getPosition());
    }
}

bool GUI::TypeBox::click(sf::Vector2i point) {
    // Checking, if pressed to start typing
    if ((backGround.getPosition().x < point.x)
        && (backGround.getPosition().y < point.y)
        && (backGround.getPosition().x + backGround.getSize().x > point.x)
        && (backGround.getPosition().y + backGround.getSize().y > point.y)) {
        selected = true;
        pressed = true;

        // Finding selection point
        if (point.x > drawText.getGlobalBounds().size.x + drawText.getPosition().x) {
            caret = drawText.getString().getSize();
        } else {
            caret = 0;
            while (point.x > drawText.findCharacterPos(caret).x) {
                caret++;
            }
        }

        selectLength = 0;
        showCaret = true;
        clock.restart();

        return false;
    } else {
        bool wasSelected = selected;
        selected = false;
        pressed = false;
        selectLength = 0;
        caret = 0;
        showCaret = false;
        clock.stop();

        return wasSelected;
    }
    debug();
}

void GUI::TypeBox::unClick() {
    pressed = false;
}

bool GUI::TypeBox::keyPress(sf::Event::KeyPressed state) {
    if (selected) {
        pressed = false;
        debug();
        const sf::String& str = drawText.getString();
        switch (state.code) {
        case sf::Keyboard::Key::Delete:
            if (selectLength) {
                // Clearing selected part
                clearSelected(str);
            } else {
                // Delete previous symbol
                if (caret < str.getSize()) {
                    drawText.setString(str.substring(0, caret) + str.substring(caret+1));
                }
            }
            break;

        case sf::Keyboard::Key::Backspace:
            if (selectLength) {
                // Clearing selected part
                clearSelected(str);
                drawCaret.setPosition(drawText.findCharacterPos(caret));
            } else {
                // Delete previous symbol
                if (caret > 0) {
                    drawText.setString(str.substring(0, caret-1) + str.substring(caret));
                    caret--;
                    drawCaret.setPosition(drawText.findCharacterPos(caret));
                }
            }
            break;

        case sf::Keyboard::Key::Left:
            if (state.shift) {
                if (caret > 0) {
                    caret--;
                    selectLength++;
                    drawCaret.setPosition(drawText.findCharacterPos(caret));
                    updateInversePos();
                }
            } else {
                if (selectLength) {
                    selectLength = 0;
                } else {
                    if (caret > 0) {
                        caret--;
                        drawCaret.setPosition(drawText.findCharacterPos(caret));
                    }
                }
            }
            break;

        case sf::Keyboard::Key::Right:
            if (state.shift) {
                if (caret < str.getSize()) {
                    caret++;
                    selectLength--;
                    drawCaret.setPosition(drawText.findCharacterPos(caret));
                    updateInversePos();
                }
            } else {
                if (selectLength) {
                    selectLength = 0;
                } else {
                    if (caret < str.getSize()) {
                        caret++;
                        drawCaret.setPosition(drawText.findCharacterPos(caret));
                    }
                }
            }
            break;

        case sf::Keyboard::Key::A:
            if (state.control) {
                // Selecting all text
                caret = str.getSize();
                selectLength = -caret;
                drawCaret.setPosition(drawText.findCharacterPos(caret));
                updateInversePos();
            }
            break;

        case sf::Keyboard::Key::C:
            if (state.control && selectLength) {
                // Copying to clipboard selected part
                if (selectLength < 0) {
                    sf::Clipboard::setString(str.substring(caret+selectLength, -selectLength));
                } else {
                    sf::Clipboard::setString(str.substring(caret, selectLength));
                }
            }
            break;

        case sf::Keyboard::Key::X:
            if (state.control && selectLength) {
                // Copying to clipboard selected part and clearing it
                if (selectLength < 0) {
                    sf::Clipboard::setString(str.substring(caret+selectLength, -selectLength));
                } else {
                    sf::Clipboard::setString(str.substring(caret, selectLength));
                }
                clearSelected(str);
                drawCaret.setPosition(drawText.findCharacterPos(caret));
            }
            break;

        case sf::Keyboard::Key::V:
            if (state.control) {
                // Copying text from clipboard
                clearSelected(str);
                const sf::String& adding = sf::Clipboard::getString();
                drawText.setString(str.substring(0, caret) + adding + str.substring(caret));
                // Moving caret
                caret += adding.getSize();
                drawCaret.setPosition(drawText.findCharacterPos(caret));
            }
            break;

        case sf::Keyboard::Key::Enter:
            // Finishing entering text
            selected = false;
            selectLength = 0;
            showCaret = false;
            clock.stop();
            return true;
        }
        debug();
    }
    return false;
}

void GUI::TypeBox::inputText(char32_t ch) {
    // Resetting
    pressed = false;

    // Checking, if current box selected and not entering special comands
    if (selected && ch > 40) {
        // Adding new charachter, deleting selected part
        const sf::String& str = drawText.getString();
        debug();
        if (selectLength < 0) {
            drawText.setString(str.substring(0, caret+selectLength) + ch + str.substring(caret));
            caret += selectLength;
        } else {
            drawText.setString(str.substring(0, caret) + ch + str.substring(caret+selectLength));
        }
        selectLength = 0;

        // Moving caret
        caret++;
        drawCaret.setPosition(drawText.findCharacterPos(caret));
    }
}

void GUI::TypeBox::update(sf::Vector2i point) {
    if (selected) {
        if (pressed) {
            // Getting new caret position and selection length
            selectLength += caret;
            if (point.x > drawText.getGlobalBounds().size.x + drawText.getPosition().x) {
                caret = drawText.getString().getSize();
            } else {
                caret = 0;
                while (point.x > drawText.findCharacterPos(caret).x) {
                    caret++;
                }
            }
            selectLength -= caret;

            debug();

            // Update caret position
            drawCaret.setPosition(drawText.findCharacterPos(caret));
            // Update inverse rect and text
            updateInversePos();
        }
        // Blinking caret
        if (clock.getElapsedTime().asSeconds() > 1) {
            showCaret ^= true;
            updateTime = clock.restart();
        }
    }
}

void GUI::TypeBox::draw(Window& window) {
    window.draw(backGround);
    window.draw(drawText);
    if (selectLength) {
        // Inversing color in rect
        window.draw(inverseRect);
        window.draw(inverseText);
    }
    if (showCaret) {
        window.draw(drawCaret);
    }
}

void GUI::TypeBox::setString(std::string _str) {
    // Resetting values
    selected = false;
    selectLength = 0;
    showCaret = false;
    clock.stop();

    // Setting new text
    drawText.setString(_str);
}

// Getting result
std::string GUI::TypeBox::getString() {
    return drawText.getString();
}

float GUI::TypeBox::getNumber() {
    // Clearing all none-number charachters
    sf::String s = drawText.getString();
    std::string outStr(s.getSize(), '\0');
    int l=0;

    // Getting corrected string number
    for (int i=0; i < s.getSize(); ++i) {
        if ((s[i] >= '0' && s[i] <= '9') || (s[i] == '.')) {
            outStr[l] = s[i];
            l++;
        }
    }

    // Getting number itself
    float number = 0;
    if (l==0) {
        number = 0;
        drawText.setString("0");
    } else {
        outStr = outStr.substr(0, l);
        number = std::stof(outStr);
        drawText.setString(outStr);
    }
    return number;

    // At this moment - simple function
    // TODO: create improved parser for understand +-*/()^
}

void GUI::TypeBox::debug() {
    #if DEBUG
    // Additional check
    std::cout << (std::string)drawText.getString() << ' ' << drawText.getString().getSize() << ' ' << caret << ' ' << selectLength << '\n';
    if (caret > drawText.getString().getSize() || caret+selectLength > drawText.getString().getSize()) {
        throw "Error";
    }
    #endif
}
