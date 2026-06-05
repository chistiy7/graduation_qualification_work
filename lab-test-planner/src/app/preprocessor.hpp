#pragma once

#include "model/scenario_bundle.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace lab {

// Препроцессор: загрузка и проверка входных данных
class Preprocessor {
public:
    [[nodiscard]] ScenarioBundle loadBuiltin(const std::string& id) const;
    [[nodiscard]] ScenarioBundle loadFromFile(const std::filesystem::path& path) const;

    [[nodiscard]] std::vector<std::string> validate(const ScenarioBundle& bundle) const;
    void ensureValid(const ScenarioBundle& bundle) const;

private:
    [[nodiscard]] bool hasCapableEquipment(const ProblemDefinition& p,
                                           const std::string& operationId) const;
};

}  // namespace lab
