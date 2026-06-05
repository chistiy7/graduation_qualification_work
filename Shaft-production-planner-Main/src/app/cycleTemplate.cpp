#include "cycleTemplate.hpp"


CycleTemplate::CycleTemplate(Window& _window)
: window(_window) {}

void CycleTemplate::getInput() {
    // Handling events
    while (const std::optional event = window.pollEvent()) {
        // Window closed: exit
        if (event->is<sf::Event::Closed>()) {
            window.close();
            running = false;
            App::stop();
            break;
        }

        // Catching pressing keys
        if (auto t = event->getIf<sf::Event::KeyPressed>()) {
            keyDown(*t);
        }
        // Catching clicking and unclicking at mouse
        if (auto t = event->getIf<sf::Event::MouseButtonPressed>()) {
            // Switching depend on pressed button, what to do
            switch (t->button) {
            case sf::Mouse::Button::Left:
                LClick(t->position);
                break;

            case sf::Mouse::Button::Right:
                RClick(t->position);
                break;
            }
        }
        if (auto t = event->getIf<sf::Event::MouseButtonReleased>()) {
            // Switching depend on pressed button, what to do
            switch (t->button) {
            case sf::Mouse::Button::Left:
                LUnClick(t->position);
                break;

            case sf::Mouse::Button::Right:
                RUnClick(t->position);
                break;
            }
        }
        // Catching clicking and unclicking at mouse
        if (auto t = event->getIf<sf::Event::TextEntered>()) {
            textInput(t->unicode);
        }
    }
}

void CycleTemplate::stop() {
    running = false;
}

void CycleTemplate::run() {
    while (running) {
        getInput();
        update();
        draw();
    }
}
