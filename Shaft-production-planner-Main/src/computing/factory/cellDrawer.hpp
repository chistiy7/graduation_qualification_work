#pragma once

#include "../cellTypes.hpp"
#include "../../app/window.hpp"

// Size of square side of cell (in pixels)
const float cellSize = 50.f;


// Class for draw textured cells
class CellDrawer {
private:
    const static unsigned spriteCount = 8;
    const sf::Texture textures[spriteCount];
    const sf::Texture warningTexture;
    sf::Sprite sprites[spriteCount];
    sf::Sprite warningSprite;
    bool drawTexture = true;

    sf::Color getColor(CellType type);
    sf::Sprite& getSprite(CellType type);

public:
    CellDrawer();
    void switchDrawType();
    void draw(Window& _window, CellType type, sf::Vector2f absPos);
    void drawWarning(Window& _window, sf::Vector2f absPos);
};
