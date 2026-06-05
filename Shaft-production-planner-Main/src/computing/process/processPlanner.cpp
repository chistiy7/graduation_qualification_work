#include "processPlanner.hpp"


ProcessPlanner::ProcessPlanner(Window& window)
: backGround({1200, 195}),
stepTexts {
    {window, getOffset(0), machineHeaderHeight, LanguagedText{"Rough", "Черновая"}},
    {window, getOffset(1), furnaceHeaderHieght, LanguagedText{"Thermal", "Термическая"}},
    {window, getOffset(2), machineHeaderHeight, LanguagedText{"Main", "Получистовая"}},
    {window, getOffset(3), furnaceHeaderHieght, LanguagedText{"Thermal", "Термическая"}},
    {window, getOffset(4), machineHeaderHeight, LanguagedText{"Trimming", "Чистовая"}},
    {window, getOffset(5), furnaceHeaderHieght, LanguagedText{"Thermal", "Термическая"}},
    {window, getOffset(6), machineHeaderHeight, LanguagedText{"Finishing", "Отделочная"}},
    {window, getOffset(7), furnaceHeaderHieght, LanguagedText{"Thermal", "Термическая"}},
},
singleTimeDescription(window, 5, singleTimeHeight, LanguagedText{"Per part:", "На деталь:"}, GUI::Aligment::Left),
singleCostDescription(window, 5, singleCostHeight, LanguagedText{"Per part:", "На деталь:"}, GUI::Aligment::Left),
machinesDescription(window, 5, latheHeight, LanguagedText{"Lathes:", "Станков:"}, GUI::Aligment::Left),
furnacesDescription(window, 5, furnaceHeight, LanguagedText{"Furnaces:", "Печей:"}, GUI::Aligment::Left),
singleTimeTexts {
    {window.font}, {window.font}, {window.font}, {window.font},
    {window.font}, {window.font}, {window.font}, {window.font},
},
singleCostTexts {
    {window.font}, {window.font}, {window.font}, {window.font},
    {window.font}, {window.font}, {window.font}, {window.font},
},
countTexts {
    {window.font}, {window.font}, {window.font}, {window.font},
    {window.font}, {window.font}, {window.font}, {window.font},
},
summaryText(window, getOffset(8)+50, machineHeaderHeight, LanguagedText{"Sumary:", "Суммарно:"}),
partTimeText(window, getOffset(8)+50, singleTimeHeight, std::format("{:.1f}", getPartProductionTime())),
partCostText(window, getOffset(8)+50, singleCostHeight, std::format("{:.1f}", getPartCost())),
machineCountText(window, getOffset(8)+50, latheHeight, std::to_string(getLatheCount())),
furnaceCountText(window, getOffset(8)+50, furnaceHeight, std::to_string(getFurnaceCount()))
{
    // Setting background
    backGround.setPosition({0, machineHeaderHeight+5});
    backGround.setFillColor(sf::Color{120, 120, 120});

    // Updating texts
    for (int i=startStep; i < endStep; ++i) {
        // Mechanical step
        singleTimeTexts[i*2].setPosition({getOffset(i*2), singleTimeHeight+15});
        singleCostTexts[i*2].setPosition({getOffset(i*2), singleCostHeight+15});
        countTexts[i*2].setPosition({getOffset(i*2), latheHeight+15});
        // Thermal step
        singleTimeTexts[i*2+1].setPosition({getOffset(i*2+1), singleTimeHeight+15});
        singleCostTexts[i*2+1].setPosition({getOffset(i*2+1), singleCostHeight+15});
        countTexts[i*2+1].setPosition({getOffset(i*2+1), furnaceHeight+15});
    }
    // Update all texts
    update();
}

constexpr float ProcessPlanner::getOffset(int index) {
    return stepOffset * index + stepStartOffset;
}

void ProcessPlanner::setTargetProduction(float _target) {
    partProductionTarget = _target;
    // Update process parameters
    recalculate();
    // Updating texts
    update();
}

void ProcessPlanner::setTargetVolume(int _target) {
    targetBatchVolume = _target;
    // Update process parameters
    recalculate();
    // Updating texts
    update();
}

std::string ProcessPlanner::getTargetProduction() {
    return std::format("{:.1f}", partProductionTarget);
}

std::string ProcessPlanner::getTargetVolume() {
    return std::to_string(targetBatchVolume);
}

void ProcessPlanner::update() {
    // Updating texts
    for (int i=startStep; i < endStep; ++i) {
        // Mechanical part
        singleTimeTexts[i*2].setString(std::format("{:.1f}", mechanicalStages[i].getTimePerOperation()));
        singleTimeTexts[i*2].setOrigin(singleTimeTexts[i*2].getGlobalBounds().size/2.0f);
        singleCostTexts[i*2].setString(std::format("{:.0f}", mechanicalStages[i].getPartCost()));
        singleCostTexts[i*2].setOrigin(singleCostTexts[i*2].getGlobalBounds().size/2.0f);
        countTexts[i*2].setString(std::to_string(mechanicalStages[i].getReqieredQuantity()));
        countTexts[i*2].setOrigin(countTexts[i*2].getGlobalBounds().size/2.0f);
        // Thermal part
        singleTimeTexts[i*2+1].setString(std::format("{:.1f}", thermalStages[i].getTimePerOperation()));
        singleTimeTexts[i*2+1].setOrigin(singleTimeTexts[i*2+1].getGlobalBounds().size/2.0f);
        singleCostTexts[i*2+1].setString(std::format("{:.0f}", thermalStages[i].getPartCost()));
        singleCostTexts[i*2+1].setOrigin(singleCostTexts[i*2+1].getGlobalBounds().size/2.0f);
        countTexts[i*2+1].setString(std::to_string(thermalStages[i].getReqieredQuantity()));
        countTexts[i*2+1].setOrigin(countTexts[i*2+1].getGlobalBounds().size/2.0f);
    }
    partTimeText.setText(std::format("{:.1f}", getPartProductionTime()));
    partCostText.setText(std::format("{:.0f}", getPartCost()));
    machineCountText.setText(std::to_string(getLatheCount()));
    furnaceCountText.setText(std::to_string(getFurnaceCount()));
}

void ProcessPlanner::draw(Window& window) {
    // Background
    window.draw(backGround);

    // Table
    singleTimeDescription.draw(window);
    singleCostDescription.draw(window);
    machinesDescription.draw(window);
    furnacesDescription.draw(window);
    for (int i=startStep*2; i < endStep*2; ++i) {
        stepTexts[i].draw(window);
        window.draw(singleTimeTexts[i]);
        window.draw(singleCostTexts[i]);
        window.draw(countTexts[i]);
    }
    
    // Summary
    summaryText.draw(window);
    partTimeText.draw(window);
    partCostText.draw(window);
    machineCountText.draw(window);
    furnaceCountText.draw(window);
}
