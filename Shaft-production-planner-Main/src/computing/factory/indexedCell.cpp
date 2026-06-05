#include "indexedCell.hpp"


IndexedCell::IndexedCell(CellType type)
: Cell(type) {}

void IndexedCell::draw(Window& window, sf::Vector2f absPos) {
    drawer.draw(window, type, absPos);
    // Draw special sign if not connected to main
    if (index > 1) {
        drawer.drawWarning(window, absPos);
    }
}

unsigned IndexedCell::getIndex() {
    return index;
}

void IndexedCell::setIndex(unsigned _index) {
    index = _index;
}
