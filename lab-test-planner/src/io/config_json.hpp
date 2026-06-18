#pragma once

#include "config/config_overrides.hpp"

#include <filesystem>

namespace lab {

// Загрузка частичных переопределений техкарт из JSON.
[[nodiscard]] LabConfigOverrides loadConfigOverridesJson(const std::filesystem::path& path);

}  // namespace lab
