// Test headless du module core : World (EnTT) + JobSystem + SnapshotBuffer.
// Aucune fonction fenetre/rendu raylib n'est appelee (Vector3/Color sont de
// simples structs). Sortie : "OK" et code 0, sinon details + code 1.

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <future>
#include <string>
#include <vector>

#include "engine/core/components.h"
#include "engine/core/frame_snapshot.h"
#include "engine/core/jobs.h"
#include "engine/core/world.h"

namespace {

int g_failures = 0;

} // namespace

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            std::fprintf(stderr, "ECHEC ligne %d : %s\n", __LINE__, #cond);  \
            ++g_failures;                                                    \
        }                                                                    \
    } while (false)

int main() {
    using namespace racer::engine;

    constexpr std::size_t kEntityCount = 100;

    // --- Creation du monde : 100 entites completes ---
    World world;
    std::vector<entt::entity> entities;
    entities.reserve(kEntityCount);
    for (std::size_t i = 0; i < kEntityCount; ++i) {
        const entt::entity entity = world.CreateEntity();
        const float fi = static_cast<float>(i);
        world.Add<TransformComponent>(entity, Vector3{fi, 0.0f, -fi}, 0.0f, 0.0f, 0.0f);
        world.Add<KinematicsComponent>(entity, fi * 0.5f, 0.0f, false);
        world.Add<RenderMeshComponent>(entity, static_cast<std::uint32_t>(i),
                                       static_cast<std::uint32_t>(i % 4),
                                       Color{static_cast<unsigned char>(i), 64, 128, 255});
        world.Add<LapProgressComponent>(entity);
        world.Add<NameComponent>(entity, "Racer " + std::to_string(i));
        if (i == 0) {
            world.Add<PlayerTag>(entity);
        } else {
            world.Add<AiTag>(entity);
        }
        entities.push_back(entity);
    }

    // --- Helpers World ---
    CHECK(world.Has<PlayerTag>(entities[0]));
    CHECK(!world.Has<AiTag>(entities[0]));
    CHECK(world.Has<AiTag>(entities[1]));
    CHECK(world.Get<NameComponent>(entities[42]).name == "Racer 42");
    CHECK(world.Registry().view<PlayerTag>().size() == 1);
    CHECK(world.Registry().view<AiTag>().size() == kEntityCount - 1);

    const entt::entity temp = world.CreateEntity();
    CHECK(world.Registry().valid(temp));
    world.DestroyEntity(temp);
    CHECK(!world.Registry().valid(temp));

    // --- JobSystem : Submit + future ---
    JobSystem jobs;
    CHECK(jobs.WorkerCount() >= 1);
    std::atomic<int> counter{0};
    std::future<void> done = jobs.Submit([&counter] { counter.fetch_add(1); });
    done.get();
    CHECK(counter.load() == 1);

    // --- ParallelFor : modifie les transforms (entites distinctes, pas de
    // changement structurel du registry -> sur) ---
    jobs.ParallelFor(0, kEntityCount, 7, [&world, &entities](std::size_t i) {
        TransformComponent& transform = world.Get<TransformComponent>(entities[i]);
        const float fi = static_cast<float>(i);
        transform.position.y = fi * 2.0f;
        transform.heading = fi * 0.01f;
        transform.roll = fi * 0.001f;
    });
    for (std::size_t i = 0; i < kEntityCount; ++i) {
        CHECK(world.Get<TransformComponent>(entities[i]).position.y == static_cast<float>(i) * 2.0f);
    }

    // ParallelFor sur intervalle vide : fn ne doit jamais etre appelee.
    jobs.ParallelFor(5, 5, 4, [](std::size_t) { CHECK(false); });

    // --- Snapshot : capture, publication, relecture ---
    SnapshotBuffer buffer;
    CHECK(buffer.ReadLatest().items.empty()); // rien de publie pour l'instant

    FrameSnapshot& write = buffer.WriteBegin();
    write.simTime = 1.25;
    CaptureSnapshot(world, write);
    buffer.Publish();

    const FrameSnapshot& read = buffer.ReadLatest();
    CHECK(&read == &write);
    CHECK(read.simTime == 1.25);
    CHECK(read.items.size() == kEntityCount);

    // L'ordre d'iteration d'EnTT n'est pas garanti : on indexe par meshId.
    std::vector<const RenderItem*> byMesh(kEntityCount, nullptr);
    for (const RenderItem& item : read.items) {
        CHECK(item.meshId < kEntityCount);
        if (item.meshId < kEntityCount) {
            CHECK(byMesh[item.meshId] == nullptr);
            byMesh[item.meshId] = &item;
        }
    }
    for (std::size_t i = 0; i < kEntityCount; ++i) {
        const RenderItem* item = byMesh[i];
        CHECK(item != nullptr);
        if (item == nullptr) {
            continue;
        }
        const float fi = static_cast<float>(i);
        CHECK(item->position.x == fi);
        CHECK(item->position.y == fi * 2.0f);
        CHECK(item->position.z == -fi);
        CHECK(item->heading == fi * 0.01f);
        CHECK(item->roll == fi * 0.001f);
        CHECK(item->materialId == static_cast<std::uint32_t>(i % 4));
        CHECK(item->tint.r == static_cast<unsigned char>(i));
        CHECK(item->tint.g == 64);
        CHECK(item->tint.b == 128);
        CHECK(item->tint.a == 255);
    }

    // --- Deuxieme frame : verifie l'alternance des deux buffers ---
    world.Get<TransformComponent>(entities[0]).position.y = 999.0f;
    FrameSnapshot& write2 = buffer.WriteBegin();
    CHECK(&write2 != &write); // l'autre buffer
    write2.simTime = 2.5;
    CaptureSnapshot(world, write2);
    buffer.Publish();

    const FrameSnapshot& read2 = buffer.ReadLatest();
    CHECK(&read2 == &write2);
    CHECK(read2.simTime == 2.5);
    CHECK(read2.items.size() == kEntityCount);
    bool foundUpdated = false;
    for (const RenderItem& item : read2.items) {
        if (item.meshId == 0) {
            foundUpdated = (item.position.y == 999.0f);
        }
    }
    CHECK(foundUpdated);

    if (g_failures != 0) {
        std::fprintf(stderr, "%d verification(s) en echec\n", g_failures);
        return 1;
    }
    std::printf("OK\n");
    return 0;
}
