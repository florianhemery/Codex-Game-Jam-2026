#
# EPITECH PROJECT, 2026
# racer
# File description:
# Core module — ECS, job system and sim/render snapshot buffering
#

# Module core : ECS (EnTT), job system et double buffering sim/rendu.

# EnTT (header-only) : recupere ici pour rester local au module core,
# aucun impact sur les autres cibles.
include(FetchContent)
FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG v3.13.2
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(entt)

add_library(engine_core STATIC
    world.cpp
    jobs.cpp
    frame_snapshot.cpp
)
target_include_directories(engine_core PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(engine_core PUBLIC raylib EnTT::EnTT)
target_compile_features(engine_core PUBLIC cxx_std_20)

# Test headless (aucune fenetre) : valide World + JobSystem + SnapshotBuffer.
add_executable(engine_core_test
    engine_core_test.cpp
)
target_link_libraries(engine_core_test PRIVATE engine_core)
# Sortie dans build-core/<config>/ (chemin attendu par la verification).
set_target_properties(engine_core_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
