#pragma once

#include "grid.hpp"


// Class representing whole workshop
class Factory : public Grid {
public:
    Factory(Window& window, sf::Vector2f pos);
    void draw(Window& window);
    void set(CellType type, sf::Vector2i absPos);
    void remove(sf::Vector2i absPos);
    void reset(sf::Vector2i absPos);
    static bool checkConnections();
    void tryOptimize();
    void updateWays();
    static int getOptimalWay();
    static CellType getOptimalWayType();
    static std::string getPathTypeText();
    void resetWayType();
    static void saveToFile(std::ofstream& fout);

private:
    static void indexConnected(int X, int Y, unsigned index);
    static int minWay;
};
