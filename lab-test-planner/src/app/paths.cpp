#include "app/paths.hpp"

#include <filesystem>

namespace lab {

namespace {

std::filesystem::path detectRoot() {
    auto cwd = std::filesystem::current_path();
    for (int i = 0; i < 6; ++i) {
        if (std::filesystem::exists(cwd / "data/scenarios")) {
            return cwd;
        }
        if (!cwd.has_parent_path()) break;
        cwd = cwd.parent_path();
    }
    return std::filesystem::current_path();
}

}  // namespace

std::filesystem::path projectRoot() { return detectRoot(); }

std::filesystem::path resourcesDir() { return projectRoot() / "resources"; }

std::filesystem::path dataDir() { return projectRoot() / "data" / "scenarios"; }

std::filesystem::path outputDir() {
    auto dir = projectRoot() / "output";
    std::filesystem::create_directories(dir);
    return dir;
}

}  // namespace lab
