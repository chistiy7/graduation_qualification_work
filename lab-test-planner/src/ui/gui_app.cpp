#include "ui/gui_app.hpp"

#include "app/paths.hpp"
#include "app/pipeline.hpp"
#include "app/preprocessor.hpp"
#include "engine/lab_optimiser.hpp"
#include "engine/layout_map.hpp"
#include "io/scenario_json.hpp"
#include "model/scenario_bundle.hpp"

#include "ui/window.hpp"

#include <SFML/Graphics.hpp>

#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace lab::ui {

namespace {

struct Button {
    sf::FloatRect rect;
    std::string label;
};

bool contains(const sf::FloatRect& r, sf::Vector2i p) {
    return r.contains(sf::Vector2f(static_cast<float>(p.x), static_cast<float>(p.y)));
}

void drawButton(AppWindow& window, const Button& btn, const sf::Font& font, sf::Color fill) {
    sf::RectangleShape box({btn.rect.size.x, btn.rect.size.y});
    box.setPosition({btn.rect.position.x, btn.rect.position.y});
    box.setFillColor(fill);
    box.setOutlineColor(sf::Color(60, 60, 80));
    box.setOutlineThickness(1.f);
    window.draw(box);

    sf::Text text(font, btn.label, 16);
    text.setFillColor(sf::Color::White);
    text.setPosition({btn.rect.position.x + 12.f, btn.rect.position.y + 8.f});
    window.draw(text);
}

enum class Screen { Input, Results };

std::string objectiveLabel(const ProblemDefinition& p) {
    return p.objectiveMode == ObjectiveMode::TotalCostRub ? "C (руб)" : "K (взвеш.)";
}

}  // namespace

void runGui() {
    AppWindow window(960, 720, "LabPlanner — испытательная лаборатория");
    Preprocessor preprocessor;
    Pipeline pipeline;

    Screen screen = Screen::Input;
    std::optional<PipelineOutput> lastRun;
    std::string statusLine = "Выберите режим программы (партия S, операции D, ячейки 2×2 м)";
    std::vector<std::string> resultLines;

    const Button btnBasic{{sf::Vector2f{40.f, 120.f}, sf::Vector2f{420.f, 36.f}},
                          "BasicMechanical"};
    const Button btnExtended{{sf::Vector2f{40.f, 162.f}, sf::Vector2f{420.f, 36.f}},
                             "MechanicalExtended"};
    const Button btnThermal{{sf::Vector2f{40.f, 204.f}, sf::Vector2f{420.f, 36.f}},
                            "ThermalCycle"};
    const Button btnThermoMech{{sf::Vector2f{40.f, 246.f}, sf::Vector2f{420.f, 36.f}},
                                 "Thermomechanical"};
    const Button btnTwo{{sf::Vector2f{40.f, 288.f}, sf::Vector2f{420.f, 36.f}},
                        "BasicMechanical (2 образца)"};
    const Button btnLoad{{sf::Vector2f{40.f, 330.f}, sf::Vector2f{420.f, 36.f}},
                         "Загрузить JSON из data/"};
    const Button btnRun{{sf::Vector2f{40.f, 390.f}, sf::Vector2f{200.f, 44.f}}, "Рассчитать"};
    const Button btnExport{{sf::Vector2f{40.f, 620.f}, sf::Vector2f{220.f, 40.f}},
                           "Экспорт отчёта"};
    const Button btnSave{{sf::Vector2f{280.f, 620.f}, sf::Vector2f{220.f, 40.f}},
                         "Сохранить JSON"};
    const Button btnBack{{sf::Vector2f{520.f, 620.f}, sf::Vector2f{160.f, 40.f}}, "Назад"};
    const Button btnQuit{{sf::Vector2f{700.f, 620.f}, sf::Vector2f{160.f, 40.f}}, "Выход"};

    ScenarioBundle pending = buildDemoSimple();

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                break;
            }
            if (auto click = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (click->button != sf::Mouse::Button::Left) continue;
                const auto pos = click->position;

                if (screen == Screen::Input) {
                    if (contains(btnBasic.rect, pos)) {
                        pending = buildDemoForMode(LabProgramMode::BasicMechanical);
                        statusLine = "Сценарий: " + pending.name;
                    } else if (contains(btnExtended.rect, pos)) {
                        pending = buildDemoForMode(LabProgramMode::MechanicalExtended);
                        statusLine = "Сценарий: " + pending.name;
                    } else if (contains(btnThermal.rect, pos)) {
                        pending = buildDemoForMode(LabProgramMode::ThermalCycle);
                        statusLine = "Сценарий: " + pending.name;
                    } else if (contains(btnThermoMech.rect, pos)) {
                        pending = buildDemoForMode(LabProgramMode::Thermomechanical);
                        statusLine = "Сценарий: " + pending.name;
                    } else if (contains(btnTwo.rect, pos)) {
                        pending = buildDemoTwoSpecimens();
                        statusLine = "Сценарий: " + pending.name;
                    } else if (contains(btnLoad.rect, pos)) {
                        pending = preprocessor.loadFromFile(dataDir() / "demo_simple.json");
                        statusLine = "Загружен из data/scenarios/";
                    } else if (contains(btnRun.rect, pos)) {
                        try {
                            lastRun = pipeline.run(pending, true);
                            resultLines.clear();
                            std::istringstream ss(
                                LabOptimiser::formatReport(lastRun->result) + "\n" +
                                renderLayoutMatrix(pending.problem, lastRun->result.optimizedRoute));
                            std::string line;
                            while (std::getline(ss, line)) {
                                resultLines.push_back(line);
                            }
                            const auto obj = objectiveLabel(pending.problem);
                            statusLine = obj + "₀=" +
                                         std::to_string(lastRun->result.comparison.baseline.K) +
                                         " " + obj + "₁=" +
                                         std::to_string(lastRun->result.comparison.optimized.K);
                            screen = Screen::Results;
                        } catch (const std::exception& ex) {
                            statusLine = std::string("Ошибка: ") + ex.what();
                        }
                    }
                } else {
                    if (contains(btnBack.rect, pos)) {
                        screen = Screen::Input;
                    } else if (contains(btnExport.rect, pos) && lastRun) {
                        statusLine = "Отчёт: " + lastRun->reportPath.string() + " | CSV: " +
                                       lastRun->csvPath.string();
                    } else if (contains(btnSave.rect, pos) && lastRun) {
                        const auto path = outputDir() / (lastRun->bundle.name + "_saved.json");
                        saveScenarioJson(path, lastRun->bundle);
                        statusLine = "Сохранено: " + path.string();
                    } else if (contains(btnQuit.rect, pos)) {
                        window.close();
                    }
                }
            }
        }

        window.clear(sf::Color(28, 32, 42));

        sf::Text title(window.font(), "LabPlanner — планирование испытательной лаборатории", 22);
        title.setFillColor(sf::Color(220, 230, 255));
        title.setPosition({40.f, 24.f});
        window.draw(title);

        sf::Text status(window.font(), statusLine, 14);
        status.setFillColor(sf::Color(180, 200, 220));
        status.setPosition({40.f, 64.f});
        window.draw(status);

        if (screen == Screen::Input) {
            drawButton(window, btnBasic, window.font(), sf::Color(70, 110, 160));
            drawButton(window, btnExtended, window.font(), sf::Color(70, 110, 160));
            drawButton(window, btnThermal, window.font(), sf::Color(70, 110, 160));
            drawButton(window, btnThermoMech, window.font(), sf::Color(70, 110, 160));
            drawButton(window, btnTwo, window.font(), sf::Color(70, 110, 160));
            drawButton(window, btnLoad, window.font(), sf::Color(70, 110, 160));
            drawButton(window, btnRun, window.font(), sf::Color(40, 140, 90));

            sf::Text hint(window.font(),
                          "Партия S, операции D, матрица состояний стендов.\n"
                          "Ячейка сетки 2×2 м; L — в шагах ячеек.\n"
                          "ЦФ: открытая постановка (C, руб) или K (взвешенная).",
                          14);
            hint.setFillColor(sf::Color(160, 170, 190));
            hint.setPosition({40.f, 460.f});
            window.draw(hint);
        } else {
            float y = 110.f;
            for (size_t i = 0; i < resultLines.size() && i < 28; ++i) {
                sf::Text line(window.font(), resultLines[i], 13);
                line.setFillColor(sf::Color(210, 215, 230));
                line.setPosition({40.f, y});
                window.draw(line);
                y += 20.f;
            }
            drawButton(window, btnExport, window.font(), sf::Color(90, 90, 140));
            drawButton(window, btnSave, window.font(), sf::Color(90, 90, 140));
            drawButton(window, btnBack, window.font(), sf::Color(100, 100, 110));
            drawButton(window, btnQuit, window.font(), sf::Color(140, 70, 70));
        }

        window.display();
    }
}

}  // namespace lab::ui
