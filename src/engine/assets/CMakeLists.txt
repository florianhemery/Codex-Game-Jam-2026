# Module assets : registre d'assets (modeles/textures) + hot-reload shaders.
add_library(engine_assets STATIC
    asset_registry.cpp
    shader_watcher.cpp
)
target_include_directories(engine_assets PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(engine_assets PUBLIC raylib)
target_compile_features(engine_assets PRIVATE cxx_std_20)
