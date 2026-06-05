#include "submenuCycle.hpp"
// Saving objects
#include "../factory/factory.hpp"
#include "../product/productProcess.hpp"


SubmenuCycle::SubmenuCycle(Window& window)
: CycleTemplate(window),
settings(window),
selectProductButton(window, 82, 2, 160, 50, {"Product", "Изделие"}),
selectProcessButton(window, 242, 2, 160, 50, {"Process", "Процесс"}),
selectFactoryButton(window, 402, 2, 160, 50, {"Factory", "Цех"}),
saveButton(window, 750, 10, 150, 40, {"Save", "Сохранить"}),
loadButton(window, 910, 10, 150, 40, {"Load", "Загрузить"}),
exportButton(window, 1070, 10, 150, 40, {"Export", "Выгрузить"}),
saveInfo(window, 580, 10, {"Saved", "Сохранено"}) {}

void SubmenuCycle::save() {
    // Create save file
    std::ofstream fout("factory.cfg");

    // Writing file name
    fout << "factory.cfg" << '\n';

    // Saving static data
    ProductProcess::save(fout);
    Factory::save(fout);
}

void SubmenuCycle::load() {
    // Openning file for loading from
    std::ifstream fin("factory.cfg");

    // Skipping first line (with file name)
    std::string line;
    std::getline(fin, line);

    // Reading data
    ProductProcess::load(fin);
    std::getline(fin, line);
    Factory::load(fin);
}

void SubmenuCycle::result() {
    // Saving results of all work to export file
    std::ofstream fout("Results.txt");

    // Writing title
    switch (LanguagedText::getLanguage()) {
    case Language::English:
        fout << "Exported production process:\n\n";
        break;
    
    case Language::Russian:
        fout << "Экспортированный производственный процесс:\n\n";
    }

    // Writing input data
    ProductProcess::saveInput(fout);
    
    // Writing calculated data
    ProductProcess::saveOutput(fout);

    // Writing field data
    Factory::saveToFile(fout);
}
