/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Headless core module test entry point
*/

#include <cstdio>
#include <vector>

#include "Engine/Core/CoreTestRunner.hpp"

int main()
{
    CoreTestRunner runner;
    racer::engine::World world;
    std::vector<entt::entity> entities;
    racer::engine::SnapshotBuffer buffer;

    runner.populateWorld(world, entities);
    runner.verifyWorldTags(world, entities);
    runner.verifyWorldLifecycle(world);
    runner.verifyJobSystem();
    runner.verifyParallelFor(world, entities);
    racer::engine::FrameSnapshot &firstWrite =
        runner.verifySnapshot(world, buffer);
    runner.verifySecondFrame(world, entities, buffer, firstWrite);

    if (runner.failures() != 0) {
        std::fprintf(stderr, "%d verification(s) en echec\n",
            runner.failures());
        return 1;
    }
    std::printf("OK\n");
    return 0;
}
