#include "cell.hpp"


// Realisation of static object in cell class
CellDrawer Cell::drawer;


Cell::Cell(CellType _type)
: type(_type) {

}

CellType Cell::getType() {
    return type;
}

void Cell::setType(CellType _type) {
    type = _type;
}

void Cell::draw(Window& window, sf::Vector2f absPos) {
    drawer.draw(window, type, absPos);
}

void Cell::draw(Window& window, sf::Vector2i absPos) {
    drawer.draw(window, type, {(float)absPos.x-cellSize/2, (float)absPos.y-cellSize/2});
}

void Cell::loadFrom(char letter) {
    switch (letter) {
    case 'v':
        type = CellType::Void;
        break;

    case 'n':
        type = CellType::None;
        break;

    case 'w':
        type = CellType::UnspecifiedWay;
        break;

    case 'c':
        type = CellType::Warehouse;
        break;

    case 'l':
        type = CellType::Lathe_1;
        break;

    case 'f':
        type = CellType::Furnace_1;
        break;
    
    }
}

char Cell::saveAs() {
    switch (type) {
    case CellType::Void:
        return 'v';

    case CellType::None:
        return 'n';

    case CellType::UnspecifiedWay:
    case CellType::ManualWay:
    case CellType::HoistWay:
    case CellType::ConveyorWay:
        return 'w';

    case CellType::Warehouse:
        return 'c';

    case CellType::Lathe_1:
        return 'l';

    case CellType::Furnace_1:
        return 'f';
    }
    // Default variant with nothing
    return ' ';
}
