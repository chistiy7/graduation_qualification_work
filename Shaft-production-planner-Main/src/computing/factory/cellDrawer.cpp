#include "cellDrawer.hpp"


CellDrawer::CellDrawer()
: textures {
    {(sf::Texture)(resourcesDir() / "machines/tile.png")},
    {(sf::Texture)(resourcesDir() / "machines/manualWay.png")},
    {(sf::Texture)(resourcesDir() / "machines/hoistWay.png")},
    {(sf::Texture)(resourcesDir() / "machines/conveyorWay.png")},
    {(sf::Texture)(resourcesDir() / "machines/unspecifiedMachine.png")},
    {(sf::Texture)(resourcesDir() / "machines/lathe.png")},
    {(sf::Texture)(resourcesDir() / "machines/furnace.png")},
    {(sf::Texture)(resourcesDir() / "machines/cargo1.png")}},
warningTexture(resourcesDir() / "machines/no-path-icon.png"),
sprites {
    {(sf::Sprite)textures[0]},
    {(sf::Sprite)textures[1]},
    {(sf::Sprite)textures[2]},
    {(sf::Sprite)textures[3]},
    {(sf::Sprite)textures[4]},
    {(sf::Sprite)textures[5]},
    {(sf::Sprite)textures[6]},
    {(sf::Sprite)textures[7]}},
warningSprite(warningTexture) {
    // Setting sizes of all sprites
    for (int i=0; i < spriteCount; ++i) {
        sprites[i].setScale({cellSize/320, cellSize/320});
    }
}

sf::Color CellDrawer::getColor(CellType type) {
    switch (type) {
    case CellType::None:
        return sf::Color::White;

    case CellType::ManualWay:
    case CellType::HoistWay:
    case CellType::ConveyorWay:
    case CellType::UnspecifiedWay:
        return sf::Color::Yellow;

    case CellType::Lathe_1:
        return sf::Color::Red;

    case CellType::Furnace_1:
        return sf::Color::Cyan;
    
    default:
        return sf::Color{100};
    }
}

sf::Sprite& CellDrawer::getSprite(CellType type) {
    return sprites[(int)type-2];
}

void CellDrawer::switchDrawType() {
    drawTexture ^= true;
}

void CellDrawer::draw(Window& window, CellType type, sf::Vector2f absPos) {
    if (drawTexture && type != CellType::None && type != CellType::Void) {
        // Drawing cell as sprite with texture
        getSprite(type).setPosition(absPos);
        window.draw(getSprite(type));
    } else {
        // Draw cell as rect with color
        sf::RectangleShape shape({cellSize, cellSize});
        shape.setFillColor(getColor(type));
        shape.setPosition(absPos);
        window.draw(shape);
    }
}

void CellDrawer::drawWarning(Window& window, sf::Vector2f absPos) {
    if (drawTexture) {
        // Draw warning sprite
        warningSprite.setPosition(absPos);
        window.draw(warningSprite);
    }
}
