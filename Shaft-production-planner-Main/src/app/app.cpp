#include "app.hpp"

// Realisation of base path
const std::filesystem::path resourcesDir() {
#if DEBUG
    return "../resources/";
#else
    return "resources/";
#endif
}

bool App::running = true;
Cycle App::nextCycle = Cycle::Default;

App::App()
: window(1200, 720, "Planner") {}

void App::stop() {
    running = false;
    nextCycle = Cycle::None
;}

void App::startNext(Cycle newCycle) {
    nextCycle = newCycle;
}
