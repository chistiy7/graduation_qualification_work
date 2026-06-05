# Setting minimum required for cmake versions
cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export compile_commands.json so editors (clangd/IntelliSense) find include paths
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Homebrew SFML (macOS Apple Silicon / Intel)
if(APPLE)
    if(EXISTS "/opt/homebrew/lib/cmake/SFML")
        list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")
    elseif(EXISTS "/usr/local/lib/cmake/SFML")
        list(APPEND CMAKE_PREFIX_PATH "/usr/local")
    endif()
endif()

project(Planner LANGUAGES CXX)

# Platform-specific flags (original project targeted Windows + static SFML)
if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc")
    set(SFML_STATIC_LIBRARIES TRUE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows -static")
endif()

# Collect sources (no .rc on Unix)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)
if(WIN32)
    file(GLOB_RECURSE WIN_SOURCES "${CMAKE_SOURCE_DIR}/src/*.rc")
    list(APPEND SOURCES ${WIN_SOURCES})
endif()

add_executable(${PROJECT_NAME} ${SOURCES})

find_package(SFML 3 COMPONENTS System Window Graphics Audio REQUIRED)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_link_libraries(${PROJECT_NAME} PRIVATE
    SFML::System
    SFML::Window
    SFML::Graphics
    SFML::Audio
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

message(STATUS "Run from project root: cd ${CMAKE_SOURCE_DIR} && ./build/Planner")
