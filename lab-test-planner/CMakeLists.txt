cmake_minimum_required(VERSION 3.16)

project(LabPlanner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(APPLE)
    if(EXISTS "/opt/homebrew/lib/cmake/SFML")
        list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")
    elseif(EXISTS "/usr/local/lib/cmake/SFML")
        list(APPEND CMAKE_PREFIX_PATH "/usr/local")
    endif()
endif()

find_package(SFML 3 COMPONENTS System Window Graphics REQUIRED)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_executable(LabPlanner ${SOURCES})
target_include_directories(LabPlanner PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(LabPlanner PRIVATE SFML::System SFML::Window SFML::Graphics)
set_target_properties(LabPlanner PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

option(LAB_PLANNER_BUILD_TESTS "Build chapter 3.6 verification test" ON)
if(LAB_PLANNER_BUILD_TESTS)
    add_executable(LabPlanner_test tests/test_route_metrics.cpp)
    target_include_directories(LabPlanner_test PRIVATE "${CMAKE_SOURCE_DIR}/src")
    target_sources(LabPlanner_test PRIVATE
        src/domain/laboratory.cpp
        src/domain/test_operation_catalog.cpp
        src/domain/stand_catalog.cpp
        src/engine/metrics.cpp
        src/engine/route_analysis.cpp
        src/engine/stand_state_matrix.cpp
        src/engine/layout_map.cpp
        src/engine/route_planner.cpp
        src/engine/lab_optimiser.cpp
        src/model/lab_program_mode.cpp
        src/model/program_builder.cpp
        src/model/scenario_bundle.cpp
    )
    add_test(NAME route_metrics COMMAND LabPlanner_test)
endif()

message(STATUS "Run GUI from project root: cd ${CMAKE_SOURCE_DIR} && ./build/LabPlanner")
