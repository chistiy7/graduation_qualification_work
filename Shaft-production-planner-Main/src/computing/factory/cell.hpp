#pragma once

#include "cellDrawer.hpp"

// Class representing single cell of grid
class Cell {
public:
    Cell(CellType type = CellType::None);
    CellType getType();
    void setType(CellType type);
    void draw(Window& window, sf::Vector2f absPos);
    void draw(Window& window, sf::Vector2i absPos);
    void loadFrom(char letter);
    char saveAs();

protected:
    // Type of selected cell
    CellType type;
    // Global devise for draw cells
    static CellDrawer drawer;
};
