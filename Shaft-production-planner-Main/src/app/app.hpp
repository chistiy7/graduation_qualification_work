#pragma once

#include "window.hpp"
#include "../cycles.hpp"

// Main process in current app
class App {
 private:
    Window window;
    static bool running;
    static Cycle nextCycle;

 public:
    App();
    void run();
    static void stop();
    static void startNext(Cycle newCycle);
};
