#pragma once

#include <fstream>
#include "indexedCell.hpp"


// Matrix n*m with cell
class Field {
private:
    // Array with all cells
    IndexedCell* grid;
    // Dimensions of grid
    unsigned width, height;

public:
    Field(unsigned width, unsigned height);
    Field(const Field& field);
    ~Field();
    Field& operator=(const Field&);
    void reset(unsigned width, unsigned height);
    void setWidth(unsigned width);
    void setHeight(unsigned height);
    unsigned getWidth();
    unsigned getHeight();
    // Functions for getting cell at need position
    IndexedCell& getCell(sf::Vector2i pos);
    IndexedCell& operator[](sf::Vector2i pos);
    IndexedCell& operator[](unsigned pos);
    void save(std::ofstream& fout);
    void load(std::ifstream& fin);
};
