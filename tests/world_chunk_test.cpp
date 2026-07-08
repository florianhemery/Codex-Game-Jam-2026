/*

** EPITECH PROJECT, 2026

** racer

** File description:

** Headless Aurélia world unit tests

*/



#include <array>

#include <cmath>

#include <cstdio>

#include <cstdlib>

#include <string>

#include <vector>



#include "Track/Track.hpp"
#include "World/Aurelia/AureliaData.hpp"

#include "World/Chunk/ChunkGenerator.hpp"

#include "World/Stream/ChunkStreamer.hpp"



namespace {



int g_failures = 0;



void expectTrue(bool cond, const char *msg)

{

    if (!cond) {

        std::fprintf(stderr, "FAIL: %s\n", msg);

        ++g_failures;

    }

}



void expectNear(float a, float b, float eps, const char *msg)

{

    if (std::fabs(a - b) > eps) {

        std::fprintf(stderr, "FAIL: %s (%.3f vs %.3f)\n", msg, a, b);

        ++g_failures;

    }

}



void testBiomeLayout()

{

    using racer::world::BiomeId;

    using racer::world::biomeForChunk;



    expectTrue(biomeForChunk(0, 0) == BiomeId::COAST, "marina chunk is coast");

    expectTrue(biomeForChunk(-1, -1) == BiomeId::FOREST, "nw is forest");

    expectTrue(biomeForChunk(1, 0) == BiomeId::PORT, "east is port");

    expectTrue(biomeForChunk(0, 1) == BiomeId::VOLCANO, "south is volcano");

}



void testStreamingRing()

{

    racer::world::ChunkStreamer streamer;

    streamer.updateCenter(16.0f, -8.0f);

    streamer.ensureLoaded();



    const auto &loaded = streamer.loadedChunks();

    expectTrue(loaded.size() == 9, "3x3 ring loads nine chunks");



    for (const auto &chunk : loaded) {

        expectTrue(chunk.generated, "chunk marked generated");

        expectTrue(!chunk.heightmap.empty(), "chunk has heightmap");

    }

}



void testIsLoaded()

{

    racer::world::ChunkStreamer streamer;

    streamer.updateCenter(16.0f, -8.0f);

    streamer.ensureLoaded();



    expectTrue(streamer.isLoaded(16.0f, -8.0f), "center loaded");

    expectTrue(!streamer.isLoaded(2048.0f, 2048.0f),

        "far world position not loaded");

}



void testHeightSampling()

{

    racer::world::ChunkStreamer streamer;

    streamer.updateCenter(16.0f, -8.0f);

    streamer.ensureLoaded();



    float marina = streamer.sampleHeight(16.0f, -8.0f);

    expectTrue(marina >= 0.0f && marina < 40.0f, "marina height plausible");



    streamer.updateCenter(-80.0f, -120.0f);

    streamer.ensureLoaded();

    float forest = streamer.sampleHeight(-80.0f, -120.0f);

    expectTrue(forest >= 2.0f, "forest garage on road surface");

}



void testForestHigherThanCoast()

{

    racer::world::ChunkData coast =

        racer::world::ChunkGenerator::generate({0, 0});

    racer::world::ChunkData forest =

        racer::world::ChunkGenerator::generate({-1, -1});



    float coastMid = coast.heightmap[static_cast<size_t>(

        (racer::world::kChunkResolution / 2) * racer::world::kChunkResolution
            + (racer::world::kChunkResolution / 2))];

    float forestMid = forest.heightmap[static_cast<size_t>(

        (racer::world::kChunkResolution / 2) * racer::world::kChunkResolution
            + (racer::world::kChunkResolution / 2))];



    expectTrue(forestMid > coastMid + 2.0f,

        "forest hills higher than marina flats");

}



void testSampleHeightMatchesGenerator()

{

    racer::world::ChunkData chunk =

        racer::world::ChunkGenerator::generate({-1, -1});

    float lx = 42.0f;

    float lz = 88.0f;

    float direct = racer::world::ChunkGenerator::sampleHeight(chunk, lx, lz);



    racer::world::ChunkStreamer streamer;

    float wx = -static_cast<float>(racer::world::kChunkSize) + lx;

    float wz = -static_cast<float>(racer::world::kChunkSize) + lz;

    streamer.updateCenter(wx, wz);

    streamer.ensureLoaded();

    float streamed = streamer.sampleHeight(wx, wz);



    expectNear(direct, streamed, 0.05f,

        "streamer height matches generator on loaded chunk");

}



void testPropsOnGround()

{

    racer::world::ChunkData chunk =

        racer::world::ChunkGenerator::generate({-1, -1});

    expectTrue(!chunk.props.empty(), "forest chunk has props");



    for (const racer::world::PropInstance &prop : chunk.props) {

        float gy = racer::world::ChunkGenerator::sampleHeight(

            chunk, prop.localX, prop.localZ);

        expectTrue(gy >= 4.0f && gy < 35.0f, "prop ground height in range");

    }

}



void testRoadPaint()

{

    racer::world::ChunkData chunk =

        racer::world::ChunkGenerator::generate({0, 0});

    bool hasAsphalt = false;

    for (racer::world::SurfaceKind sk : chunk.splat) {

        if (sk == racer::world::SurfaceKind::ASPHALT) {

            hasAsphalt = true;

            break;

        }

    }

    expectTrue(hasAsphalt, "marina chunk has painted roads");

}



void testChunkSeam()
{
    using racer::world::ChunkGenerator;
    using racer::world::kChunkResolution;

    racer::world::ChunkData west =
        ChunkGenerator::generate({0, 0});
    racer::world::ChunkData east =
        ChunkGenerator::generate({1, 0});

    int edge = kChunkResolution - 1;
    for (int iz = 0; iz < kChunkResolution; ++iz) {
        float hWest = west.heightmap[static_cast<size_t>(
            iz * kChunkResolution + edge)];
        float hEast = east.heightmap[static_cast<size_t>(
            iz * kChunkResolution)];
        expectNear(hWest, hEast, 0.01f, "chunk seam height matches");
    }
}

void testCalderaSinglePeak()
{
    using racer::world::ChunkGenerator;
    using racer::world::kCalderaCenterX;
    using racer::world::kCalderaCenterZ;
    using racer::world::kChunkSize;

    float peak = ChunkGenerator::sampleWorldHeight(
        kCalderaCenterX, kCalderaCenterZ);
    float offCenter = ChunkGenerator::sampleWorldHeight(
        kCalderaCenterX + kChunkSize, kCalderaCenterZ);
    float otherChunk = ChunkGenerator::sampleWorldHeight(
        kChunkSize + 64.0f, kCalderaCenterZ + kChunkSize);

    expectTrue(peak > offCenter + 4.0f,
        "caldera peak higher than offset sample");
    expectTrue(peak > otherChunk + 2.0f,
        "single caldera not repeated per chunk");
}

void testRoadSmoothness()
{
    using racer::world::AureliaData;
    using racer::world::ChunkGenerator;
    using racer::world::ChunkStreamer;

    const auto &graph = AureliaData::roadGraph();
    ChunkStreamer streamer;

    for (size_t ei = 0; ei < graph.edges().size(); ++ei) {
        float prevH = 0.0f;
        bool hasPrev = false;
        float distAcc = 0.0f;
        Vector2 prevP{0.0f, 0.0f};

        for (int s = 0; s <= 64; ++s) {
            float t = static_cast<float>(s) / 64.0f;
            Vector2 p = graph.pointOnEdge(static_cast<int>(ei), t);
            streamer.updateCenter(p.x, p.y);
            streamer.ensureLoaded();
            float h = streamer.sampleHeight(p.x, p.y);

            if (hasPrev) {
                float dx = p.x - prevP.x;
                float dz = p.y - prevP.y;
                distAcc += std::sqrt(dx * dx + dz * dz);
                if (distAcc >= 8.0f) {
                    expectTrue(std::fabs(h - prevH) <= 1.5f,
                        "road height jump within 1.5m over 8m");
                    prevH = h;
                    prevP = p;
                    distAcc = 0.0f;
                }
            } else {
                prevH = h;
                prevP = p;
                hasPrev = true;
            }
        }
    }
}

void testTerrainNormalsFaceUp()
{
    racer::world::ChunkData chunk =
        racer::world::ChunkGenerator::generate({0, 0});
    const int quads = racer::world::kChunkResolution - 1;

    auto corner = [&](int cx, int cz) {
        float lx = static_cast<float>(cx) / static_cast<float>(quads)
            * racer::world::kChunkSize;
        float lz = static_cast<float>(cz) / static_cast<float>(quads)
            * racer::world::kChunkSize;
        int idx = cz * racer::world::kChunkResolution + cx;
        float h = chunk.heightmap[static_cast<size_t>(idx)];
        return std::array<float, 3>{lx, h, lz};
    };

    auto crossY = [](const std::array<float, 3> &a,
                      const std::array<float, 3> &b,
                      const std::array<float, 3> &c) {
        float abx = b[0] - a[0];
        float aby = b[1] - a[1];
        float abz = b[2] - a[2];
        float acx = c[0] - a[0];
        float acy = c[1] - a[1];
        float acz = c[2] - a[2];
        return abz * acx - abx * acz;
    };

    auto p00 = corner(0, 0);
    auto p10 = corner(1, 0);
    auto p01 = corner(0, 1);
    auto p11 = corner(1, 1);

    expectTrue(crossY(p00, p11, p10) > 0.0f, "terrain tri A faces up");
    expectTrue(crossY(p00, p01, p11) > 0.0f, "terrain tri B faces up");
}

void testRaceLabelsStable()

{

    std::vector<racer::TrackDef> presets(5);

    for (int i = 0; i < 5; ++i) {

        presets[static_cast<size_t>(i)].name = "Circuit Test " + std::to_string(i);

    }

    racer::world::AureliaData::attachRaceLabels(presets);



    for (const racer::world::PoiInstance &poi :

        racer::world::AureliaData::worldPois()) {

        if (poi.type != racer::world::PoiType::RACE_ENTRY) {

            continue;

        }

        expectTrue(poi.label != nullptr, "race label non-null");

        expectTrue(poi.label[0] != '\0', "race label non-empty");

    }

}



} // namespace



int main()

{

    testBiomeLayout();

    testStreamingRing();

    testIsLoaded();

    testHeightSampling();

    testForestHigherThanCoast();

    testSampleHeightMatchesGenerator();

    testPropsOnGround();

    testRoadPaint();
    testChunkSeam();
    testCalderaSinglePeak();
    testRoadSmoothness();
    testTerrainNormalsFaceUp();

    testRaceLabelsStable();



    if (g_failures > 0) {

        std::fprintf(stderr, "%d test(s) failed\n", g_failures);

        return 1;

    }

    std::printf("OK: all %d Aurelia world checks passed\n", 13);

    return 0;

}

