add_executable(voxel_client
    src/main.cpp
)

# Note : PAS de server_core ici, meme transitivement dans les includes -- voir
# transports/loopback/CMakeLists.txt (le lien vers server_core y est PRIVATE).
target_link_libraries(voxel_client PRIVATE
    common
    loopback_transport
    raylib
)
target_compile_features(voxel_client PRIVATE cxx_std_20)
