#pragma once

#include "field.hpp"
#include "../../app/cycleTemplate.hpp"
#include "../process.hpp"


// Class for draw and calculate objects at the grid
class Grid {
private:
    // Absolute position of first element
    const sf::Vector2f pos;
    // Counter indicators
    GUI::DynamicText latheCountText, furnaceCountText, warehouseCountText;
    // Warnings
    GUI::WarningBox latheCountHigh, furnaceCountHigh, warehouseCountHigh;
    GUI::WarningBox latheCountLow, furnaceCountLow, warehouseCountLow;

protected:
    // Main field [height*width], filled with cells
    static Field field;
    // Counters of current objects at field
    static int latheCount, furnaceCount, warehouseCount;
    // Function for recalculate counts of machines after loading
    static void recalculateCellsCount();
    // Function for update texts
    void updateLatheText();
    void updateFurnaceText();
    void updateWarehouseText();

    // Getting local position (in cells) from absolute coordinate
    sf::Vector2i getLocal(sf::Vector2i absPos);
    // Getting absolute position (in pixels) from local coordinate
    sf::Vector2f getAbs(sf::Vector2i localPos);

public:
    Grid(Window& window, sf::Vector2f position);
    void blit(Window& window);
    bool isSelected(sf::Vector2i pos);
    static void setWidth(unsigned width);
    static void setHeight(unsigned height);
    static std::string getWidth();
    static std::string getHeight();
    static void save(std::ofstream& fout);
    static void load(std::ifstream& fin);
};
