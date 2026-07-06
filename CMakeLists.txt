cmake_minimum_required(VERSION 3.24)
project(racer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra)
endif()

include(FetchContent)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raylib)

add_executable(racer
    src/main.cpp
    src/vehicle/car.cpp
    src/track/track.cpp
    src/ai/ai_driver.cpp
    src/race/race_state.cpp
    src/render/track_renderer.cpp
    src/render/car_renderer.cpp
    src/render/hud.cpp
)

target_include_directories(racer PRIVATE src)
target_link_libraries(racer PRIVATE raylib)
target_compile_features(racer PRIVATE cxx_std_20)

# Outil headless : simule une course complete (IA vs IA) pour valider la
# logique de jeu sans fenetre ni pilotage manuel.
add_executable(race_sim_debug
    src/tools/race_sim_debug.cpp
    src/vehicle/car.cpp
    src/track/track.cpp
    src/ai/ai_driver.cpp
    src/race/race_state.cpp
)
target_include_directories(race_sim_debug PRIVATE src)
target_link_libraries(race_sim_debug PRIVATE raylib)
target_compile_features(race_sim_debug PRIVATE cxx_std_20)
