#include "factory.hpp"
#include "optimiser.hpp"


int Factory::minWay = 0;

Factory::Factory(Window& window, sf::Vector2f _pos)
: Grid(window, _pos) {
    // First updation of existing grid
    checkConnections();
}

void Factory::draw(Window& window) {
    // Draw back part with grid
    blit(window);
}

void Factory::set(CellType _type, sf::Vector2i absPos) {
    // If selected cell is void - resetting to none
    if (field.getCell(getLocal(absPos)).getType() == CellType::Void) {
        field.getCell(getLocal(absPos)).setType(CellType::None);
    } else {
        // Reset, if set on existing cell
        switch (field.getCell(getLocal(absPos)).getType()) {
        // Update counters
        case CellType::Lathe_1:
            latheCount--;
            updateLatheText();
            break;

        case CellType::Furnace_1:
            furnaceCount--;
            updateFurnaceText();
            break;

        case CellType::Warehouse:
            warehouseCount--;
            updateWarehouseText();
            break;
        }
        // Setting selected cell to matched type
        field.getCell(getLocal(absPos)).setType(_type);
        // Increase counters
        switch (_type) {
        case CellType::Lathe_1:
            latheCount++;
            updateLatheText();
            break;

        case CellType::Furnace_1:
            furnaceCount++;
            updateFurnaceText();
            break;

        case CellType::Warehouse:
            warehouseCount++;
            updateWarehouseText();
            break;
        }
        // Update linkage of field
        checkConnections();
    }
}

void Factory::remove(sf::Vector2i absPos) {
    switch (field.getCell(getLocal(absPos)).getType()) {
    case CellType::Void:
        // Special action in void cell - can't remove that doesn't exist
        return;

    case CellType::None:
        // Special action in void none - none in building
        field.getCell(getLocal(absPos)).setType(CellType::Void);
        // Update linkage of field
        checkConnections();
        return;

    // Update counters
    case CellType::Lathe_1:
        latheCount--;
        updateLatheText();
        break;

    case CellType::Furnace_1:
        furnaceCount--;
        updateFurnaceText();
        break;

    case CellType::Warehouse:
        warehouseCount--;
        updateWarehouseText();
        break;
    }
    // In any normal machine/way - reset to none-type
    field.getCell(getLocal(absPos)).setType(CellType::None);
    // Update linkage of field
    checkConnections();
}

void Factory::reset(sf::Vector2i absPos) {
    switch (field.getCell(getLocal(absPos)).getType()) {
    case CellType::Void:
        // Special action in void cell - reset to none type
        field.getCell(getLocal(absPos)).setType(CellType::None);
        break;
    }
}

bool Factory::checkConnections() {
    unsigned indexCounter = 1;
    bool notConnected = false;

    // Resetting all indexes
    for (int y=0; y < field.getHeight(); ++y) {
        for (int x=0; x < field.getWidth(); ++x) {
            field.getCell({x, y}).setIndex(0);
        }
    }

    // Checking grid on way networks
    for (int y=0; y < field.getHeight(); ++y) {
        for (int x=0; x < field.getWidth(); ++x) {
            if (field.getCell({x, y}).getIndex() == 0) {
                switch (field[{x, y}].getType()) {
                    case CellType::UnspecifiedWay:
                    case CellType::ManualWay:
                    case CellType::HoistWay:
                    case CellType::ConveyorWay:
                    // Setting cell and all, connected to it to be same index
                    indexConnected(x, y, indexCounter);
                    if (indexCounter != 1) {
                        notConnected = true;
                    }
                    indexCounter++;
                }
            }
        }
    }

    // Checking on not connected machines
    for (int y=0; y < field.getHeight(); ++y) {
        for (int x=0; x < field.getWidth(); ++x) {
            if (field[{x, y}].getIndex() == 0) {
                switch (field[{x, y}].getType()) {
                case CellType::Lathe_1:
                case CellType::Furnace_1:
                    field.getCell({x, y}).setIndex(indexCounter);
                    indexCounter++;
                    notConnected = true;
                    break;
                }
            }
        }
    }
    return notConnected;
}

void Factory::indexConnected(int X, int Y, unsigned _index) {
    // Checking, cell already have index
    if (field[{X, Y}].getIndex()) {
        return;
    }
    // Update index
    
    switch (field[{X, Y}].getType()) {
    case CellType::None:
    case CellType::Void:
        return;
    
    case CellType::UnspecifiedWay:
    case CellType::ManualWay:
    case CellType::HoistWay:
    case CellType::ConveyorWay:
        field[{X, Y}].setIndex(_index);
        if (X > 0) {
            indexConnected(X-1, Y, _index);
        }
        if (X < field.getWidth()-1) {
            indexConnected(X+1, Y, _index);
        }
        if (Y > 0) {
            indexConnected(X, Y-1, _index);
        }
        if (Y < field.getHeight()-1) {
            indexConnected(X, Y+1, _index);
        }
        break;

    default:
        field[{X, Y}].setIndex(_index);
    }
}

void Factory::tryOptimize() {
    Optimiser optimiser;
    optimiser.optimise(field);
    checkConnections();
    recalculateCellsCount();
    updateLatheText();
    updateFurnaceText();
    updateWarehouseText();
}

void Factory::updateWays() {
    // Costs of all variants
    float costs[3] {0};

    float cellCosts[3] {
        0,
        10000,
        100000,
    };

    float cellCostsPrtTime[3] {
        1000,
        500,
        200,
    };

    // Counting path tiles
    int pathCounts = 0;
    for (int y=0; y < field.getHeight(); ++y) {
        for (int x=0; x < field.getWidth(); ++x) {
            switch (field.getCell({x, y}).getType()) {
            case CellType::UnspecifiedWay:
            case CellType::ManualWay:
            case CellType::HoistWay:
            case CellType::ConveyorWay:
                pathCounts++;
                break;
            }
        }
    }

    float time = Process::getVolumeProductionTime();
    float mass = Process::getInputPart().mass;

    // Calculate cost
    // Placing object itself
    costs[0] += cellCosts[0]*pathCounts;
    costs[1] += cellCosts[1]*pathCounts;
    costs[2] += cellCosts[2]*pathCounts;

    // Calculating cost of using
    costs[0] += pathCounts*time*cellCostsPrtTime[0]*mass/10;
    costs[1] += pathCounts*time*cellCostsPrtTime[1];
    costs[2] += pathCounts*time*cellCostsPrtTime[2];

    // Check on part weight
    if (mass > 20) {
        costs[0] = 1000000000;
    }

    // Finding type with minimal cost
    int minCost = 1000000000;
    minWay = 0;
    for (int i=1; i < 4; ++i) {
        if (costs[i-1] < minCost) {
            minCost = costs[i-1];
            minWay = i;
        }
    }
    // Selecting way
    CellType wayType = getOptimalWayType();

    // Changing all ways to specified
    for (int y=0; y < field.getHeight(); ++y) {
        for (int x=0; x < field.getWidth(); ++x) {
            switch (field.getCell({x, y}).getType()) {
            case CellType::UnspecifiedWay:
            case CellType::ManualWay:
            case CellType::HoistWay:
            case CellType::ConveyorWay:
                field.getCell({x, y}).setType(wayType);
                break;
            }
        }
    }
}

int Factory::getOptimalWay() {
    return minWay;
}

CellType Factory::getOptimalWayType() {
    switch (minWay) {
    case 1:
        return CellType::ManualWay;
    
    case 2:
        return CellType::HoistWay;

    case 3:
        return CellType::ConveyorWay;
    }
    return CellType::UnspecifiedWay;
}

std::string Factory::getPathTypeText() {
    switch (LanguagedText::getLanguage()) {
    case Language::English:
        switch (minWay) {
        case 0:
            return "Transport unspecified";

        case 1:
            return "Carrying manualy";

        case 2:
            return "Transport by hoist";

        case 3:
            return "Transport by conveyor";
        }
        break;
    
    case Language::Russian:
        switch (minWay) {
        case 0:
            return "Транспорт не указан";

        case 1:
            return "Перенос вручную";

        case 2:
            return "Транспорт талью";

        case 3:
            return "Транспорт конвейером";
        }
    }
    return "";
}

void Factory::resetWayType() {
    if (minWay != 0) {
        minWay = 0;
        for (int y=0; y < field.getHeight(); ++y) {
            for (int x=0; x < field.getWidth(); ++x) {
                switch (field.getCell({x, y}).getType()) {
                case CellType::UnspecifiedWay:
                case CellType::ManualWay:
                case CellType::HoistWay:
                case CellType::ConveyorWay:
                    field.getCell({x, y}).setType(CellType::UnspecifiedWay);
                    break;
                }
            }
        }
    }
}

void Factory::saveToFile(std::ofstream& fout) {
    switch (LanguagedText::getLanguage()) {
    case Language::English:
        // Topic
        fout << "Field data:\n";

        // Save objects counts
        fout << "Machine counts:\n";
        fout << std::format("Lathes: {}, requered {};\n", latheCount, Process::getLatheCount());
        fout << std::format("Furnaces: {}, requered {};\n", furnaceCount, Process::getFurnaceCount());
        fout << std::format("Warehouces: {}, requered {};\n", warehouseCount, Process::getWarehouseCount());
        break;
    
    case Language::Russian:
        // Topic
        fout << "Данные цеха:\n";

        // Save objects counts
        fout << "Количество машин:\n";
        fout << std::format("Токарные станки: {}, необходимо {};\n", latheCount, Process::getLatheCount());
        fout << std::format("Печи: {}, необходимо {};\n", furnaceCount, Process::getFurnaceCount());
        fout << std::format("Склады: {}, необходимо {};\n", warehouseCount, Process::getWarehouseCount());
    }

    // Path tiles
    fout << '\n';
    fout << getPathTypeText() << '\n';
}
