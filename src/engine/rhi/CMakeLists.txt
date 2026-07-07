# Module RHI : abstraction GPU (Device) + render graph. Voir README.md.
add_library(engine_rhi STATIC
    device.cpp
    render_graph.cpp
)
target_include_directories(engine_rhi PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(engine_rhi PUBLIC raylib)
target_compile_features(engine_rhi PRIVATE cxx_std_20)
