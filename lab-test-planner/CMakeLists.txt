cmake_minimum_required(VERSION 3.16)

project(LabPlanner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Ядро (без main / Qt) ---
file(GLOB_RECURSE LAB_CORE_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
list(FILTER LAB_CORE_SOURCES EXCLUDE REGEX ".*/app/main\\.cpp$")
list(FILTER LAB_CORE_SOURCES EXCLUDE REGEX ".*/qt/.*")

add_library(lab_core STATIC ${LAB_CORE_SOURCES})
target_include_directories(lab_core PUBLIC "${CMAKE_SOURCE_DIR}/src")

# --- Консольный LabPlanner ---
add_executable(LabPlanner src/app/main.cpp)
target_link_libraries(LabPlanner PRIVATE lab_core)
set_target_properties(LabPlanner PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# --- Qt 6 GUI (.app) ---
option(LAB_PLANNER_BUILD_QT_GUI "Build Qt6 GUI (lab-test-planner-gui.app)" ON)
if(LAB_PLANNER_BUILD_QT_GUI)
    set(CMAKE_AUTOMOC ON)
    find_package(Qt6 REQUIRED COMPONENTS Widgets Concurrent)
    add_executable(lab-test-planner-gui MACOSX_BUNDLE
        src/qt/main.cpp
        src/qt/calculation_worker.cpp
        src/qt/calculation_worker.hpp
        src/qt/mainwindow.cpp
        src/qt/mainwindow.hpp
        src/qt/pages/input_page.cpp
        src/qt/pages/input_page.hpp
        src/qt/pages/results_page.cpp
        src/qt/pages/results_page.hpp
        src/qt/widgets/layout_grid_widget.cpp
        src/qt/widgets/layout_grid_widget.hpp
    )
    set_target_properties(lab-test-planner-gui PROPERTIES
        OUTPUT_NAME "lab-test-planner-gui"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.lab.planner.gui"
        MACOSX_BUNDLE_BUNDLE_NAME "Lab Planner"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    )
    target_link_libraries(lab-test-planner-gui PRIVATE lab_core Qt6::Widgets Qt6::Concurrent)
endif()

# --- Тесты ---
option(LAB_PLANNER_BUILD_TESTS "Build verification tests" ON)
if(LAB_PLANNER_BUILD_TESTS)
    add_executable(LabPlanner_test tests/test_route_metrics.cpp)
    target_include_directories(LabPlanner_test PRIVATE "${CMAKE_SOURCE_DIR}/src")
    target_link_libraries(LabPlanner_test PRIVATE lab_core)
    add_test(NAME route_metrics COMMAND LabPlanner_test)
endif()

message(STATUS "Console:  ${CMAKE_BINARY_DIR}/LabPlanner --cli")
if(LAB_PLANNER_BUILD_QT_GUI)
    message(STATUS "Qt GUI:   ${CMAKE_BINARY_DIR}/lab-test-planner-gui.app")
endif()
