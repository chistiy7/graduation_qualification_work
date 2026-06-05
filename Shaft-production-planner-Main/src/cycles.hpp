#pragma once


// Types of posible cycles to run
enum class Cycle {
    None,
    ProductPlanner,
    ProcessPlanner,
    FactoryPlaner,
    Default = ProductPlanner,
};
