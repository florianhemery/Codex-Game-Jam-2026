#
# EPITECH PROJECT, 2026
# racer
# File description:
# Render module — high-level HDR rendering pipeline
#

# Module rendu haut niveau (pipeline HDR, ombres, ciel, post-process).
add_library(engine_render STATIC
    render_pipeline.cpp
)
target_include_directories(engine_render PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(engine_render PUBLIC raylib engine_rhi engine_assets)
target_compile_features(engine_render PRIVATE cxx_std_20)

# Demo autonome du pipeline : fenetre de test + captures d'ecran automatiques.
add_executable(render_demo render_demo.cpp)
target_include_directories(render_demo PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(render_demo PRIVATE engine_render)
target_compile_features(render_demo PRIVATE cxx_std_20)
