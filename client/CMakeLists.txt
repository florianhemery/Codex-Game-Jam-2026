add_executable(voxel_client
    src/main.cpp
    src/render/chunk_mesher.cpp
    src/render/chunk_renderer.cpp
    src/input/voxel_raycast.cpp
    src/ui/hud.cpp
)
target_include_directories(voxel_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Note : PAS de server_core ici, meme transitivement dans les includes -- voir
# transports/loopback/CMakeLists.txt (le lien vers server_core y est PRIVATE).
target_link_libraries(voxel_client PRIVATE
    common
    loopback_transport
    raylib
)
target_compile_features(voxel_client PRIVATE cxx_std_20)
