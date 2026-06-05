#include "grid.hpp"


// Static class values
Field Grid::field{10, 5};
int Grid::latheCount = 0;
int Grid::furnaceCount = 0;
int Grid::warehouseCount = 0;


Grid::Grid(Window& window, sf::Vector2f _pos)
: pos(_pos),
latheCountText(window, 280, 50, {"Lathes: {}, needed: {}", "Станков: {}, необходимо: {}"}, GUI::Aligment::Left),
furnaceCountText(window, 280, 85, {"Furnaces: {}, needed: {}", "Печей: {}, необходимо: {}"}, GUI::Aligment::Left),
warehouseCountText(window, 280, 120, {"Warehouses: {}, needed: {}", "Складов: {}, необходимо: {}"}, GUI::Aligment::Left),
latheCountHigh(window, 740, 50, {"Number of machines is excessive", "Число станков избыточно"}, "machines/warning-icon.png"),
furnaceCountHigh(window, 740, 85, {"Number of furnaces is excessive", "Число печей избыточно"}, "machines/warning-icon.png"),
warehouseCountHigh(window, 740, 120, {"Number of warehouses is excessive", "Число складов избыточно"}, "machines/warning-icon.png"),
latheCountLow(window, 740, 50,  {"Number of machines is insufficient", "Число станков недостаточно"}, "machines/warning-icon.png"),
furnaceCountLow(window, 740, 85, {"Number of furnaces is insufficient", "Число печей недостаточно"}, "machines/warning-icon.png"),
warehouseCountLow(window, 740, 120, {"Number of warehouses is insufficient", "Число складов недостаточно"}, "machines/warning-icon.png") {
    // Placing texts
    updateLatheText();
    updateFurnaceText();
    updateWarehouseText();
}

sf::Vector2i Grid::getLocal(sf::Vector2i absPos) {
    return {int((absPos.x-pos.x)/cellSize), int((absPos.y-pos.y)/cellSize)};
}

sf::Vector2f Grid::getAbs(sf::Vector2i localPos) {
    return {localPos.x*cellSize + pos.x, localPos.y*cellSize + pos.y};
}

void Grid::updateLatheText() {
    // Update text itself
    latheCountText.setValues(latheCount, Process::getLatheCount());
    // Update warnings
    latheCountHigh.deactivate();
    latheCountLow.deactivate();
    if (latheCount > Process::getLatheCount()) {
        latheCountHigh.activate();
    } else if (latheCount < Process::getLatheCount()) {
        latheCountLow.activate();
    }
}

void Grid::updateFurnaceText() {
    // Update text itself
    furnaceCountText.setValues(furnaceCount, Process::getFurnaceCount());
    // Update warnings
    furnaceCountHigh.deactivate();
    furnaceCountLow.deactivate();
    if (furnaceCount > Process::getFurnaceCount()) {
        furnaceCountHigh.activate();
    } else if (furnaceCount < Process::getFurnaceCount()) {
        furnaceCountLow.activate();
    }
}

void Grid::updateWarehouseText() {
    // Update text itself
    warehouseCountText.setValues(warehouseCount, Process::getWarehouseCount());
    // Update warnings
    warehouseCountHigh.deactivate();
    warehouseCountLow.deactivate();
    if (warehouseCount > Process::getWarehouseCount()) {
        warehouseCountHigh.activate();
    } else if (warehouseCount < Process::getWarehouseCount()) {
        warehouseCountLow.activate();
    }
}

void Grid::recalculateCellsCount() {
    // Resetting counts
    latheCount=0;
    furnaceCount=0;
    warehouseCount=0;

    // Couting all objects
    for (int i=0; i < field.getHeight()*field.getWidth(); ++i) {
        switch (field[i].getType()) {
        case CellType::Lathe_1:
            latheCount++;
            break;

        case CellType::Furnace_1:
            furnaceCount++;
            break;

        case CellType::Warehouse:
            warehouseCount++;
            break;
        
        default:
            break;
        }
    }
}

void Grid::blit(Window& window) {
    sf::RectangleShape square({cellSize, cellSize});
    square.setPosition(pos);

    // Draw counters
    latheCountText.draw(window);
    furnaceCountText.draw(window);
    warehouseCountText.draw(window);
    latheCountHigh.draw(window);
    furnaceCountHigh.draw(window);
    warehouseCountHigh.draw(window);
    latheCountLow.draw(window);
    furnaceCountLow.draw(window);
    warehouseCountLow.draw(window);

    // Draw cells
    for (int y=0; y < field.getHeight(); ++y) {
        for (int x=0; x < field.getWidth(); ++x) {
            // Checking, if cell
            if (field.getCell({x, y}).getType() != CellType::Void) {
                field.getCell({x, y}).draw(window, getAbs({x, y}));
            }
            square.move({cellSize, 0});
        }
        square.move({-cellSize*field.getWidth(), cellSize});
    }

    // Draw vertical lines
    sf::Vertex vertLine[] {
        {pos, sf::Color::Black, {0.f, 0.f}},
        {{pos.x, pos.y + cellSize*field.getHeight()}, sf::Color::Black, {0.f, 0.f}}
    };
    for (int x=0; x < field.getWidth()+1; ++x) {
        window.draw(vertLine, 2, sf::PrimitiveType::Lines);
        vertLine[0].position.x += cellSize;
        vertLine[1].position.x += cellSize;
    }

    // Draw horizontal lines
    sf::Vertex horLine[] {
        {pos, sf::Color::Black, {0.f, 0.f}},
        {{pos.x + cellSize*field.getWidth(), pos.y}, sf::Color::Black, {0.f, 0.f}}
    };
    for (int y=0; y < field.getHeight()+1; ++y) {
        window.draw(horLine, 2, sf::PrimitiveType::Lines);
        horLine[0].position.y += cellSize;
        horLine[1].position.y += cellSize;
    }
}

bool Grid::isSelected(sf::Vector2i point) {
    return (pos.x < point.x)
    && (pos.y < point.y)
    && (pos.x + field.getWidth()*cellSize > point.x)
    && (pos.y + field.getHeight()*cellSize > point.y);
}

void Grid::setWidth(unsigned _width) {
    field.setWidth(_width);
}

void Grid::setHeight(unsigned _height) {
    field.setHeight(_height);
}

std::string Grid::getWidth() {
    return std::to_string(field.getWidth());
}

std::string Grid::getHeight() {
    return std::to_string(field.getHeight());
}

void Grid::save(std::ofstream& fout) {
    // Writing system data
    fout << '\n';
    fout << "Grid\n";
    field.save(fout);
}

void Grid::load(std::ifstream& fin) {
    // Current reading line
    std::string line;

    // Skipping first 2 lines (empty and title)
    std::getline(fin, line);
    std::getline(fin, line);

    // Load data to field
    field.load(fin);
    recalculateCellsCount();
}
