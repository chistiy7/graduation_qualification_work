#include "io/config_json.hpp"

#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace lab {

namespace {

std::string readFile(const std::filesystem::path& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("cannot read config: " + path.string());
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

std::optional<double> parseDouble(const std::string& text, const std::string& key) {
    const std::string needle = "\"" + key + "\"";
    const auto pos = text.find(needle);
    if (pos == std::string::npos) return std::nullopt;
    auto colon = text.find(':', pos + needle.size());
    if (colon == std::string::npos) return std::nullopt;
    size_t i = colon + 1;
    while (i < text.size() && std::isspace(static_cast<unsigned char>(text[i]))) ++i;
    size_t j = i;
    while (j < text.size() && (std::isdigit(static_cast<unsigned char>(text[j])) || text[j] == '.' ||
                               text[j] == '-' || text[j] == 'e' || text[j] == 'E' ||
                               text[j] == '+')) {
        ++j;
    }
    if (j == i) return std::nullopt;
    return std::stod(text.substr(i, j - i));
}

std::optional<int> parseInt(const std::string& text, const std::string& key) {
    if (auto v = parseDouble(text, key)) return static_cast<int>(*v);
    return std::nullopt;
}

std::optional<std::string> parseString(const std::string& text, const std::string& key) {
    const std::string needle = "\"" + key + "\"";
    const auto pos = text.find(needle);
    if (pos == std::string::npos) return std::nullopt;
    auto colon = text.find(':', pos + needle.size());
    if (colon == std::string::npos) return std::nullopt;
    auto q1 = text.find('"', colon + 1);
    if (q1 == std::string::npos) return std::nullopt;
    auto q2 = text.find('"', q1 + 1);
    if (q2 == std::string::npos) return std::nullopt;
    return text.substr(q1 + 1, q2 - q1 - 1);
}

std::optional<bool> parseBool(const std::string& text, const std::string& key) {
    const std::string needle = "\"" + key + "\"";
    const auto pos = text.find(needle);
    if (pos == std::string::npos) return std::nullopt;
    auto colon = text.find(':', pos + needle.size());
    if (colon == std::string::npos) return std::nullopt;
    const auto tail = text.substr(colon + 1, 12);
    if (tail.find("true") != std::string::npos) return true;
    if (tail.find("false") != std::string::npos) return false;
    return std::nullopt;
}

TestProgramOverride parseProgramBlock(const std::string& block) {
    TestProgramOverride o;
    if (auto v = parseString(block, "equipmentId")) o.equipmentId = *v;
    if (auto v = parseString(block, "operationId")) o.operationId = *v;
    if (auto v = parseDouble(block, "normativeTimeMin")) o.normativeTimeMin = *v;
    if (auto v = parseDouble(block, "modePowerKw")) o.modePowerKw = *v;
    if (auto v = parseDouble(block, "setupTimeMin")) o.setupTimeMin = *v;
    if (auto v = parseDouble(block, "setupPowerKw")) o.setupPowerKw = *v;
    if (auto v = parseInt(block, "cyclesCount")) o.cyclesCount = *v;
    if (auto v = parseDouble(block, "frequencyHz")) o.frequencyHz = *v;
    if (auto v = parseDouble(block, "cycleTimeMin")) o.cycleTimeMin = *v;
    if (auto v = parseDouble(block, "heatingTimeMin")) o.heatingTimeMin = *v;
    if (auto v = parseDouble(block, "holdingTimeMin")) o.holdingTimeMin = *v;
    if (auto v = parseDouble(block, "coolingTimeMin")) o.coolingTimeMin = *v;
    if (auto v = parseDouble(block, "sigmaMpa")) o.sigmaMpa = *v;
    if (auto v = parseDouble(block, "deltaT_C")) o.deltaT_C = *v;
    if (auto v = parseBool(block, "deformationEnergyUsed")) o.deformationEnergyUsed = *v;
    return o;
}

}  // namespace

LabConfigOverrides loadConfigOverridesJson(const std::filesystem::path& path) {
    const std::string text = readFile(path);
    LabConfigOverrides out;

    const std::string programsKey = "\"programs\"";
    const auto programsPos = text.find(programsKey);
    if (programsPos == std::string::npos) return out;

    const auto arrStart = text.find('[', programsPos);
    if (arrStart == std::string::npos) return out;

    size_t i = arrStart + 1;
    while (i < text.size()) {
        const auto objStart = text.find('{', i);
        if (objStart == std::string::npos) break;
        const auto objEnd = text.find('}', objStart);
        if (objEnd == std::string::npos) break;

        auto block = parseProgramBlock(text.substr(objStart, objEnd - objStart + 1));
        if (!block.equipmentId.empty() && !block.operationId.empty()) {
            out.programs.push_back(std::move(block));
        }

        i = objEnd + 1;
        if (text.find(']', i) < text.find('{', i)) break;
    }

    return out;
}

}  // namespace lab
