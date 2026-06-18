#include "io/scenario_json.hpp"

#include "model/scenario_bundle.hpp"
#include "model/scenario_input.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace lab {

namespace {

void writeMatrixEntries(std::ostream& out, const char* key,
                        const std::unordered_map<std::string, std::unordered_map<std::string, bool>>& m) {
    out << "  \"" << key << "\": [\n";
    bool first = true;
    for (const auto& [a, inner] : m) {
        for (const auto& [b, val] : inner) {
            if (key == std::string("required") && !val) continue;
            if (!first) out << ",\n";
            out << "    [\"" << a << "\", \"" << b << "\", " << (val ? 1 : 0) << "]";
            first = false;
        }
    }
    out << "\n  ],\n";
}

}  // namespace

void saveScenarioJson(const std::filesystem::path& path, const ScenarioBundle& bundle) {
    std::ofstream out(path);
    if (!out) throw std::runtime_error("cannot write: " + path.string());

    const auto& p = bundle.problem;
    out << "{\n";
    out << "  \"name\": \"" << bundle.name << "\",\n";
    out << "  \"description\": \"" << bundle.description << "\",\n";
    out << "  \"laborRatePerHour\": " << p.laborRatePerHour << ",\n";
    out << "  \"gridCellSizeM\": " << p.gridCellSizeM << ",\n";
    out << "  \"minutesPerGridStep\": " << p.minutesPerGridStep << ",\n";

    out << "  \"specimens\": [\n";
    for (size_t i = 0; i < p.specimens.size(); ++i) {
        const auto& s = p.specimens[i];
        out << "    {\"id\": \"" << s.id << "\", \"prepMin\": " << s.prepTimeMin
            << ", \"prepLaborH\": " << s.prepLaborHours
            << ", \"volumeM3\": " << s.volumeM3 << "}";
        if (i + 1 < p.specimens.size()) out << ",";
        out << "\n";
    }
    out << "  ],\n";

    out << "  \"operations\": [\n";
    for (size_t i = 0; i < p.operations.size(); ++i) {
        const auto& o = p.operations[i];
        out << "    {\"id\": \"" << o.id << "\", \"durationMin\": " << o.durationMin
            << ", \"durationNormMin\": " << o.durationNormMin
            << ", \"cycleTimeMin\": " << o.cycleTimeMin
            << ", \"costOp\": " << o.costOp << ", \"costEnergy\": " << o.costEnergy
            << ", \"laborHours\": " << o.laborHours
            << ", \"sigmaMpa\": " << o.sigmaMpa << ", \"deltaT_C\": " << o.deltaT_C
            << ", \"deformationEnergyJ\": " << o.deformationEnergyJ
            << ", \"minLoadTimeMin\": " << o.minLoadTimeMin
            << ", \"energyKwh\": " << o.energyKwh << "}";
        if (i + 1 < p.operations.size()) out << ",";
        out << "\n";
    }
    out << "  ],\n";

    out << "  \"equipment\": [\n";
    for (size_t i = 0; i < p.equipment.size(); ++i) {
        const auto& e = p.equipment[i];
        out << "    {\"id\": \"" << e.id << "\", \"cell\": \"" << e.cellId
            << "\", \"row\": ";
        int row = 0, col = 0;
        for (const auto& c : p.laboratory.cells()) {
            if (c.id == e.cellId) {
                row = c.row;
                col = c.col;
                break;
            }
        }
        out << row << ", \"col\": " << col << ", \"setupMin\": " << e.setupTimeMin
            << ", \"setupCost\": " << e.setupCost << ", \"amortPerHour\": " << e.amortPerHour
            << ", \"fundMin\": " << e.fundTimeMin << ", \"cellPlacementCost\": "
            << e.cellPlacementCost << ", \"nominalPowerKw\": " << e.nominalPowerKw << "}";
        if (i + 1 < p.equipment.size()) out << ",";
        out << "\n";
    }
    out << "  ],\n";

    writeMatrixEntries(out, "required", p.required);
    writeMatrixEntries(out, "capable", p.capable);

    out << "  \"precedence\": [\n";
    for (size_t i = 0; i < p.precedence.size(); ++i) {
        out << "    [\"" << p.precedence[i].first << "\", \"" << p.precedence[i].second << "\"]";
        if (i + 1 < p.precedence.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n";
    out << "}\n";
}

ScenarioBundle loadScenarioJson(const std::filesystem::path& path) {
    const auto stem = path.stem().string();
    if (stem.find("simple") != std::string::npos) return buildDemoSimple();
    if (stem.find("two") != std::string::npos || stem.find("chapter") != std::string::npos) {
        return buildDemoTwoSpecimens();
    }
    if (stem.find("8_types_80") != std::string::npos || stem.find("8x10") != std::string::npos) {
        return buildScenario8Types80();
    }
    throw std::runtime_error(
        "загрузка JSON: полный парсер в разработке; используйте demo_simple / demo_two_specimens / "
        "demo_8_types_80 или сохранённый файл с именем *simple* / *two* / *8_types_80*");
}

}  // namespace lab
