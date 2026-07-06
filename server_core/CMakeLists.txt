add_library(server_core STATIC
    src/server_core.cpp
    src/chunk_generator.cpp
    src/world_manager.cpp
)

target_include_directories(server_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(server_core PUBLIC common)
target_compile_features(server_core PUBLIC cxx_std_20)
