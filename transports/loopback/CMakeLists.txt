add_library(loopback_transport STATIC
    src/loopback_transport.cpp
)

target_include_directories(loopback_transport PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(loopback_transport
    PUBLIC common
    PRIVATE server_core
)
target_compile_features(loopback_transport PUBLIC cxx_std_20)
