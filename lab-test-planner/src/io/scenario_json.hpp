#pragma once

#include "model/scenario_bundle.hpp"

#include <filesystem>

namespace lab {

void saveScenarioJson(const std::filesystem::path& path, const ScenarioBundle& bundle);
ScenarioBundle loadScenarioJson(const std::filesystem::path& path);

}  // namespace lab
