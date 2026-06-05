#pragma once

#include <fstream>
#include "factory.hpp"


class Optimiser {
private:
    // Placing ways step
    void placeWay(Field& field, unsigned count);
    bool connectCell(Field& copy, unsigned index, int& counter);
    bool findWay(Field& copy, int& step, int& findCellX, int& findCellY, unsigned& findIndex, unsigned startIndex);
    void setCell(Field& field, int X, int Y, unsigned weight);
    bool trySetWay(Field& field, int X, int Y, unsigned weight, int& counter);
    void tryPlaceMachine(Field& field, int X, int Y, int& counter);

    // Placing objects step
    void placeObjects(Field& field);
    void findFirst(Field& field, std::vector<sf::Vector2i>& cells);
    bool placeMachine(Field& field, int X, int Y, std::vector<sf::Vector2i>& cells, std::vector<unsigned>& counts, int& counter);

    // Clearing step
    void clearRestCells(Field& field);
    void checkNotUsing(Field& field, int X, int Y, int& ways, int& machines);

    #if DEBUG
    std::ofstream fout{"cell connection.txt"};
    void debug(Field& field);
    #endif

public:
    Optimiser();
    void optimise(Field& field);
};
