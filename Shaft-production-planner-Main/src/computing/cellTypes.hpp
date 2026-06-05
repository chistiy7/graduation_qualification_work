#pragma once

// Array representing types of equipment
enum class CellType {
    // Type representing unasseseble area
    Void,

    // Type representing empty space
    None,

    // Types of transporting
    UnspecifiedWay,
    ManualWay,
    HoistWay,
    ConveyorWay,

    // Types of machines
    UnspecifiedMachine,
    Lathe_1,
    Furnace_1,

    // Types for storage
    Warehouse,
};
