#include "productCycle.hpp"


ProductCycle::ProductCycle(Window& window)
: SubmenuCycle(window),
process(window),
blankPicture(0, 60, "machines/blankBlueprint.png"),
blankWidthInput(window, 250, 205, 100, process.getBlankLength()),
blankHeightInput(window, 620, 110, 80, process.getBlankDiameter()),
blankRoughnessInput(window, 620, 65, 80, process.getBlankRoughness()),
partPicture(0, 260, "machines/shaftBlueprint.png"),
partWidthInput(window, 250, 410, 100, process.getTargetLength()),
partHeightInput(window, 655, 315, 80, process.getTargetDiameter()),
partRoughnessInput(window, 655, 265, 80, process.getTargetRoughness()),
materialText(window, 880, 60, LanguagedText{"Material:", "Материал:"}),
materialSwitch(window, 740, 100, (LanguagedText[]) {
    {"Steel", "Сталь"},
    {"Alloys", "Цветные сплавы"},
    {"Heat-resistant steel", "Жаропрочная сталь"}},
process.getMaterial())
{}

void ProductCycle::LClick(sf::Vector2i pos) {
    // Checking global objects
    if (selectProcessButton.isClicked(pos)) {
        App::startNext(Cycle::ProcessPlanner);
        stop();
        return;
    }
    if (selectFactoryButton.isClicked(pos)) {
        App::startNext(Cycle::FactoryPlaner);
        stop();
        return;
    }
    if (settings.click(pos)) {
        stop();
        return;
    }
    if (saveButton.isClicked(pos)) {
        save();
        saveInfo.reset();
        return;
    }
    if (loadButton.isClicked(pos)) {
        load();
        // Restarting cycle
        stop();
        return;
    }
    if (exportButton.isClicked(pos)) {
        result();
        saveInfo.reset();
        return;
    }

    // Check, if stop input - update values
    if (partWidthInput.click(pos)) {
        process.setTargetLength(partWidthInput.getNumber());
    }
    if (partHeightInput.click(pos)) {
        process.setTargetDiameter(partHeightInput.getNumber());
    }
    if (partRoughnessInput.click(pos)) {
        process.setTargetRoughness(partRoughnessInput.getNumber());
    }
    if (blankWidthInput.click(pos)) {
        process.setBlankLength(blankWidthInput.getNumber());
    }
    if (blankHeightInput.click(pos)) {
        process.setBlankDiameter(blankHeightInput.getNumber());;
    }
    if (blankRoughnessInput.click(pos)) {
        process.setBlankRoughness(blankRoughnessInput.getNumber());
    }
    if (materialSwitch.click(pos)) {
        process.setMaterial(materialSwitch.getValue());
    }
}

void ProductCycle::LUnClick(sf::Vector2i pos) {
    partWidthInput.unClick();
    partHeightInput.unClick();
    partRoughnessInput.unClick();
    blankWidthInput.unClick();
    blankHeightInput.unClick();
    blankRoughnessInput.unClick();
}

void ProductCycle::RClick(sf::Vector2i pos) {
    // Pass
}

void ProductCycle::keyDown(sf::Event::KeyPressed state) {
    // Check, if stop input - update values
    if (partWidthInput.keyPress(state)) {
        process.setTargetLength(partWidthInput.getNumber());
    }
    if (partHeightInput.keyPress(state)) {
        process.setTargetDiameter(partHeightInput.getNumber());
    }
    if (partRoughnessInput.keyPress(state)) {
        process.setTargetRoughness(partRoughnessInput.getNumber());
    }
    if (blankWidthInput.keyPress(state)) {
        process.setBlankLength(blankWidthInput.getNumber());
    }
    if (blankHeightInput.keyPress(state)) {
        process.setBlankDiameter(blankHeightInput.getNumber());;
    }
    if (blankRoughnessInput.keyPress(state)) {
        process.setBlankRoughness(blankRoughnessInput.getNumber());
    }
}

void ProductCycle::textInput(char32_t keyCode) {
    partWidthInput.inputText(keyCode);
    partHeightInput.inputText(keyCode);
    partRoughnessInput.inputText(keyCode);
    blankWidthInput.inputText(keyCode);
    blankHeightInput.inputText(keyCode);
    blankRoughnessInput.inputText(keyCode);
}

void ProductCycle::update() {
    partWidthInput.update(sf::Mouse::getPosition(window));
    partHeightInput.update(sf::Mouse::getPosition(window));
    partRoughnessInput.update(sf::Mouse::getPosition(window));
    blankWidthInput.update(sf::Mouse::getPosition(window));
    blankHeightInput.update(sf::Mouse::getPosition(window));
    blankRoughnessInput.update(sf::Mouse::getPosition(window));
}

void ProductCycle::draw() {
    // Draw global objects
    window.clear(sf::Color(20, 20, 20));
    selectProductButton.draw(window);
    selectProcessButton.draw(window);
    selectFactoryButton.draw(window);
    saveButton.draw(window);
    loadButton.draw(window);
    exportButton.draw(window);
    saveInfo.draw(window);

    // Draw main part
    partPicture.draw(window);
    partWidthInput.draw(window);
    partHeightInput.draw(window);
    partRoughnessInput.draw(window);
    blankPicture.draw(window);
    blankWidthInput.draw(window);
    blankHeightInput.draw(window);
    blankRoughnessInput.draw(window);
    materialText.draw(window);
    materialSwitch.draw(window);

    process.draw(window);

    settings.draw(window);

    // Display things on screen
    window.display();
}
