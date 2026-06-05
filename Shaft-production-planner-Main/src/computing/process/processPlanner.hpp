#pragma once

#include "../global/submenuCycle.hpp"
#include "../process.hpp"


// 
class ProcessPlanner : public Process {
private:
    // Grphical constants and macroses
    const float stepOffset = 110, stepStartOffset = 190;
    constexpr float getOffset(int index);
    const float machineHeaderHeight = 90, furnaceHeaderHieght = 120;
    const float singleTimeHeight = 150, singleCostHeight = 180;
    const float latheHeight = 210, furnaceHeight = 240;

    // Graphical part
    sf::RectangleShape backGround;
    GUI::Text singleTimeDescription, singleCostDescription;
    GUI::Text machinesDescription, furnacesDescription;
    GUI::Text stepTexts[8];
    sf::Text singleTimeTexts[8];
    sf::Text singleCostTexts[8];
    sf::Text countTexts[8];
    GUI::Text summaryText;
    GUI::Text partTimeText, partCostText, machineCountText, furnaceCountText;

protected:
    void update();

public:
    ProcessPlanner(Window& window);
    void setTargetProduction(float target);
    void setTargetVolume(int target);
    std::string getTargetProduction();
    std::string getTargetVolume();
    void draw(Window& window);
};
