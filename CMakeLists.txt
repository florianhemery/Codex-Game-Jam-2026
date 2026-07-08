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

option(USE_AURELIA "Enable Aurélia open world (replaces legacy hub)" ON)

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

# LLVM-MinGW lie libc++/libunwind en dynamique : copier les DLL a cote des .exe
get_filename_component(RACER_LLVM_MINGW_BIN "${CMAKE_CXX_COMPILER}" DIRECTORY)
set(RACER_RUNTIME_DLLS
    "${RACER_LLVM_MINGW_BIN}/libc++.dll"
    "${RACER_LLVM_MINGW_BIN}/libunwind.dll"
    "${RACER_LLVM_MINGW_BIN}/libwinpthread-1.dll"
)

function(racer_bundle_runtime_dlls target)
    if(NOT WIN32)
        return()
    endif()
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${RACER_RUNTIME_DLLS}
            $<TARGET_FILE_DIR:${target}>
        COMMENT "Copy LLVM-MinGW runtime DLLs for ${target}"
    )
endfunction()

# Modules moteur (Phase 0) -- libs statiques independantes, chacune possede
# son propre CMakeLists. Ne pas ajouter de sources moteur ici.
add_subdirectory(src/Engine/Rhi)
add_subdirectory(src/Engine/Assets)
add_subdirectory(src/Engine/Core)
add_subdirectory(src/Engine/Render)

set(RACER_TRACK_RENDER_SOURCES
    src/Render/Track/TrackRenderer.cpp
    src/Render/Track/TrackMeshBuilder.cpp
    src/Render/Track/TrackMeshStrip.cpp
    src/Render/Track/TrackMeshFinish.cpp
    src/Render/Track/TrackMeshGeometry.cpp
    src/Render/Track/TrackDecorBuilder.cpp
    src/Render/Track/TrackDecorClouds.cpp
    src/Render/Track/TrackDecorProps.cpp
    src/Render/Track/TrackDecorBarriers.cpp
    src/Render/Track/TrackDrawPass.cpp
    src/Render/Track/TrackDrawProps.cpp
    src/Render/Track/TrackDrawNpc.cpp
    src/Render/Track/TrackSkidMarks.cpp
    src/Render/Track/TrackSkyDraw.cpp
)

set(RACER_CAR_RENDER_SOURCES
    src/Render/Car/CarRenderer.cpp
    src/Render/Car/CarDraw.cpp
    src/Render/Car/CarPrimitives.cpp
    src/Render/Car/CarBodyDraw.cpp
    src/Render/Car/CarBodyPanels.cpp
    src/Render/Car/CarWheelDraw.cpp
)

set(RACER_HUD_SOURCES
    src/Render/Hud/Hud.cpp
    src/Render/Hud/HudGfx.cpp
    src/Render/Hud/HudMinimap.cpp
    src/Render/Hud/HudOpenWorldNav.cpp
    src/Render/Hud/HudMenu.cpp
    src/Render/Hud/HudRaceGauge.cpp
    src/Render/Hud/HudRaceOverlay.cpp
    src/Render/Hud/HudFinishScreen.cpp
)

set(RACER_VFX_SOURCES
    src/Render/Vfx/VfxSystem.cpp
    src/Render/Vfx/VfxParticlePool.cpp
    src/Render/Vfx/VfxParticleTick.cpp
    src/Render/Vfx/VfxDrawPass.cpp
    src/Render/Vfx/VfxTextureFactory.cpp
)

set(RACER_WORLD_SOURCES
    src/World/Chunk/ChunkTypes.cpp
    src/World/Chunk/ChunkGenerator.cpp
    src/World/Aurelia/AureliaTypes.cpp
    src/World/Aurelia/AureliaData.cpp
    src/World/Aurelia/AureliaWorld.cpp
    src/World/Road/RoadGraph.cpp
    src/World/Stream/ChunkStreamer.cpp
    src/World/Sim/PlayerDriveSystem.cpp
    src/World/Sim/ProgressionState.cpp
    src/World/Sim/MissionSystem.cpp
    src/World/Sim/TrafficSystem.cpp
    src/World/Sim/TuningSystem.cpp
    src/Save/SaveSystem.cpp
    src/Render/World/WorldRenderer.cpp
    src/Render/World/WorldDecorDraw.cpp
    src/Render/World/WorldPropBuilder.cpp
)

set(RACER_TRACK_DOMAIN_SOURCES
    src/Track/Track.cpp
    src/Track/TrackPresets.cpp
    src/Track/TrackWaypoints.cpp
)

set(RACER_AUDIO_SOURCES
    src/Audio/AudioSystem.cpp
)

set(RACER_RENDER_SOURCES
    ${RACER_TRACK_RENDER_SOURCES}
    ${RACER_CAR_RENDER_SOURCES}
    ${RACER_HUD_SOURCES}
    ${RACER_VFX_SOURCES}
)

add_executable(racer
    src/main.cpp
    src/App/MainApp.cpp
    src/App/GameLoop.cpp
    src/App/GameLoopVfx.cpp
    src/App/GameLoopWorld.cpp
    src/App/GameLoopInput.cpp
    src/App/AudioFeedback.cpp
    src/App/CameraController.cpp
    src/App/LapTimer.cpp
    src/App/RacerColors.cpp
    src/Engine/Input/InputBindings.cpp
    src/Vehicle/Car.cpp
    ${RACER_TRACK_DOMAIN_SOURCES}
    src/Ai/AiDriver.cpp
    src/Race/RaceState.cpp
    src/Race/RaceContactResolver.cpp
    ${RACER_RENDER_SOURCES}
    ${RACER_WORLD_SOURCES}
    ${RACER_AUDIO_SOURCES}
)

if(USE_AURELIA)
    target_compile_definitions(racer PRIVATE USE_AURELIA=1)
endif()

target_include_directories(racer PRIVATE src)
target_link_libraries(racer PRIVATE raylib engine_render)
target_compile_features(racer PRIVATE cxx_std_20)
racer_bundle_runtime_dlls(racer)

# Outil headless : simule une course complete (IA vs IA) pour valider la
# logique de jeu sans fenetre ni pilotage manuel.
add_executable(race_sim_debug
    src/Tools/RaceSimDebug.cpp
    src/Tools/RaceSimPrinter.cpp
    src/Vehicle/Car.cpp
    ${RACER_TRACK_DOMAIN_SOURCES}
    src/Ai/AiDriver.cpp
    src/Race/RaceState.cpp
    src/Race/RaceContactResolver.cpp
)
target_include_directories(race_sim_debug PRIVATE src)
target_link_libraries(race_sim_debug PRIVATE raylib)
target_compile_features(race_sim_debug PRIVATE cxx_std_20)
racer_bundle_runtime_dlls(race_sim_debug)

# Demos autonomes par sous-systeme : fenetre courte + captures d'ecran
# automatiques, utilisees par les agents pour valider visuellement leur
# travail sans lancer le jeu complet.
function(racer_add_demo name)
    add_executable(${name} ${ARGN})
    target_include_directories(${name} PRIVATE src)
    target_link_libraries(${name} PRIVATE raylib)
    target_compile_features(${name} PRIVATE cxx_std_20)
    racer_bundle_runtime_dlls(${name})
endfunction()

racer_add_demo(car_demo
    src/Render/Demos/CarDemo.cpp
    ${RACER_CAR_RENDER_SOURCES}
    src/Vehicle/Car.cpp
)
racer_add_demo(track_demo
    src/Render/Demos/TrackDemo.cpp
    ${RACER_TRACK_RENDER_SOURCES}
    ${RACER_TRACK_DOMAIN_SOURCES}
)
racer_add_demo(vfx_demo
    src/Render/Demos/VfxDemo.cpp
    ${RACER_VFX_SOURCES}
)
racer_add_demo(hud_demo
    src/Render/Demos/HudDemo.cpp
    ${RACER_HUD_SOURCES}
    src/Race/RaceState.cpp
    src/Race/RaceContactResolver.cpp
    src/Ai/AiDriver.cpp
    src/Vehicle/Car.cpp
    ${RACER_TRACK_DOMAIN_SOURCES}
)

add_executable(world_chunk_test
    tests/world_chunk_test.cpp
    src/World/Chunk/ChunkTypes.cpp
    src/World/Chunk/ChunkGenerator.cpp
    src/World/Aurelia/AureliaTypes.cpp
    src/World/Aurelia/AureliaData.cpp
    src/World/Stream/ChunkStreamer.cpp
    src/World/Road/RoadGraph.cpp
)
target_include_directories(world_chunk_test PRIVATE src)
target_link_libraries(world_chunk_test PRIVATE raylib)
target_compile_features(world_chunk_test PRIVATE cxx_std_20)
racer_bundle_runtime_dlls(world_chunk_test)

# Outil headless : conduit reellement la voiture joueur a travers le monde
# ouvert Aurelia (via PlayerDriveSystem, le meme chemin d'execution que le
# jeu) et verifie des invariants de trajectoire -- pas de saut de hauteur
# brutal (route qui devient une falaise/un canyon), pas de NaN, pas de
# sortie des WorldBounds, vitesse moyenne plausible. Complementaire de
# world_chunk_test (qui echantillonne le terrain de facon statique) et de
# race_sim_debug (circuit ferme, IA vs IA) : celui-ci couvre le monde
# ouvert en conduite reelle.
add_executable(regression_harness
    tests/regression_harness.cpp
    src/Vehicle/Car.cpp
    src/World/Chunk/ChunkTypes.cpp
    src/World/Chunk/ChunkGenerator.cpp
    src/World/Aurelia/AureliaTypes.cpp
    src/World/Aurelia/AureliaData.cpp
    src/World/Stream/ChunkStreamer.cpp
    src/World/Road/RoadGraph.cpp
    src/World/Sim/PlayerDriveSystem.cpp
)
target_include_directories(regression_harness PRIVATE src)
target_link_libraries(regression_harness PRIVATE raylib)
target_compile_features(regression_harness PRIVATE cxx_std_20)
racer_bundle_runtime_dlls(regression_harness)

racer_add_demo(world_editor
    src/Tools/WorldEditor.cpp
    src/World/Chunk/ChunkTypes.cpp
    src/World/Chunk/ChunkGenerator.cpp
    src/World/Aurelia/AureliaTypes.cpp
    src/World/Aurelia/AureliaData.cpp
    src/World/Stream/ChunkStreamer.cpp
    src/World/Road/RoadGraph.cpp
)
