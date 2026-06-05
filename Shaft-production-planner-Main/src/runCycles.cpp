#include "app/app.hpp"
#include "computing/product/productCycle.hpp"
#include "computing/process/processCycle.hpp"
#include "computing/factory/factoryCycle.hpp"

// Main process of selecting cycle
void App::run() {
    while (running) {
        switch (nextCycle) {
        case Cycle::ProductPlanner:
            {
                ProductCycle cycle(window);
                cycle.run();
            }
            break;

        case Cycle::ProcessPlanner:
            {
                ProcessCycle cycle(window);
                cycle.run();
            }
            break;

        case Cycle::FactoryPlaner:
            {
                FactoryCycle cycle(window);
                cycle.run();
            }
            break;

        case Cycle::None:
        default:
            stop();
            break;
        }
    }
}
