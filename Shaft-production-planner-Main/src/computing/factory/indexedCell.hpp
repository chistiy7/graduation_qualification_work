#pragma once

#include "cell.hpp"


// Specified cell, which nown, in to which network it connected
class IndexedCell : public Cell {
private:
    unsigned index = 0;  // 0 index represent none-cennction, other - index of network

public:
    IndexedCell(CellType type = CellType::None);
    void draw(Window& window, sf::Vector2f absPos);
    unsigned getIndex();
    void setIndex(unsigned index);
    unsigned weight;
};
