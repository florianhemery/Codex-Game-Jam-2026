#
# EPITECH PROJECT, 2026
# racer
# File description:
# Root CMake build configuration
#

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

# Modules moteur (Phase 0) -- libs statiques independantes, chacune possede
# son propre CMakeLists. Ne pas ajouter de sources moteur ici.
add_subdirectory(src/engine/rhi)
add_subdirectory(src/engine/assets)
add_subdirectory(src/engine/core)
add_subdirectory(src/engine/render)

add_executable(racer
    src/main.cpp
    src/vehicle/car.cpp
    src/track/track.cpp
    src/ai/ai_driver.cpp
    src/race/race_state.cpp
    src/render/track_renderer.cpp
    src/render/car_renderer.cpp
    src/render/hud.cpp
    src/render/vfx.cpp
)

target_include_directories(racer PRIVATE src)
target_link_libraries(racer PRIVATE raylib engine_render)
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

# Demos autonomes par sous-systeme : fenetre courte + captures d'ecran
# automatiques, utilisees par les agents pour valider visuellement leur
# travail sans lancer le jeu complet.
function(racer_add_demo name)
    add_executable(${name} ${ARGN})
    target_include_directories(${name} PRIVATE src)
    target_link_libraries(${name} PRIVATE raylib)
    target_compile_features(${name} PRIVATE cxx_std_20)
endfunction()

racer_add_demo(car_demo
    src/render/car_demo.cpp
    src/render/car_renderer.cpp
    src/vehicle/car.cpp
)
racer_add_demo(track_demo
    src/render/track_demo.cpp
    src/render/track_renderer.cpp
    src/track/track.cpp
)
racer_add_demo(vfx_demo
    src/render/vfx_demo.cpp
    src/render/vfx.cpp
)
racer_add_demo(hud_demo
    src/render/hud_demo.cpp
    src/render/hud.cpp
    src/race/race_state.cpp
    src/ai/ai_driver.cpp
    src/vehicle/car.cpp
    src/track/track.cpp
)
