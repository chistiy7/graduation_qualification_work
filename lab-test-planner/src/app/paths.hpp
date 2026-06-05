#pragma once

#include <filesystem>

namespace lab {

std::filesystem::path projectRoot();
std::filesystem::path resourcesDir();
std::filesystem::path dataDir();
std::filesystem::path outputDir();

}  // namespace lab
