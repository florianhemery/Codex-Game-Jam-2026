add_executable(worldgen_debug
    main.cpp
)

# Cet outil a le droit d'inclure server_core directement : il ne simule pas un
# client, il sert a debugger server_core en isolation (headless, sans raylib).
target_link_libraries(worldgen_debug PRIVATE common server_core)
target_compile_features(worldgen_debug PRIVATE cxx_std_20)
