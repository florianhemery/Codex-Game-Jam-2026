#
# EPITECH PROJECT, 2026
# racer
# File description:
# RHI module — GPU abstraction and render graph
#

# Module RHI : abstraction GPU (Device) + render graph. Voir README.md.
add_library(engine_rhi STATIC
    Device.cpp
    DeviceFramebuffer.cpp
    RenderGraph.cpp
)
target_include_directories(engine_rhi PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(engine_rhi PUBLIC raylib)
target_compile_features(engine_rhi PRIVATE cxx_std_20)
