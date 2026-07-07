/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Track mesh geometry helpers
*/

#include "Render/Track/TrackMeshBuilder.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <vector>

#include "rlgl.h"

namespace racer {

constexpr int kSkidTextureSize = 2048;

Vector2 TrackMeshBuilder::computePerpendicularAt(
    const std::vector<Vector2> &wp, size_t i, size_t n)
{
    Vector2 prev = wp[(i + n - 1) % n];
    Vector2 cur = wp[i];
    Vector2 next = wp[(i + 1) % n];
    Vector2 dirA{cur.x - prev.x, cur.y - prev.y};
    Vector2 dirB{next.x - cur.x, next.y - cur.y};

    normalizeVector2(dirA);
    normalizeVector2(dirB);
    Vector2 avg{dirA.x + dirB.x, dirA.y + dirB.y};

    normalizeVector2(avg);
    return Vector2{-avg.y, avg.x};
}

void TrackMeshBuilder::normalizeVector2(Vector2 &v)
{
    float len = std::sqrt(v.x * v.x + v.y * v.y);

    if (len > 1e-6f) {
        v.x /= len;
        v.y /= len;
    }
}

std::vector<Vector2> TrackMeshBuilder::computePerpendiculars(
    const Track &track)
{
    const auto &wp = track.waypoints();
    size_t n = wp.size();
    std::vector<Vector2> perp(n);

    for (size_t i = 0; i < n; ++i)
        perp[i] = computePerpendicularAt(wp, i, n);
    return perp;
}

uint32_t TrackMeshBuilder::hashIndex(size_t i)
{
    uint32_t h = static_cast<uint32_t>(i) * 2654435761u;

    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}

unsigned char TrackMeshBuilder::lerpChannel(
    unsigned char x, unsigned char y, float t)
{
    float xf = static_cast<float>(x);
    float yf = static_cast<float>(y);
    float out = xf + (yf - xf) * t;

    return static_cast<unsigned char>(out);
}

Color TrackMeshBuilder::lerpColor(Color a, Color b, float t)
{
    return Color{
        lerpChannel(a.r, b.r, t),
        lerpChannel(a.g, b.g, t),
        lerpChannel(a.b, b.b, t),
        lerpChannel(a.a, b.a, t),
    };
}

Color TrackMeshBuilder::asphaltColor(uint32_t h, SurfaceStyle style)
{
    int noise = static_cast<int>(h % 37);

    if (style == SurfaceStyle::ABIMEE) {
        unsigned char base = static_cast<unsigned char>(95 + noise);

        return Color{
            base,
            static_cast<unsigned char>(base - 4),
            static_cast<unsigned char>(base - 8),
            255,
        };
    }
    unsigned char base = static_cast<unsigned char>(52 + noise % 18);

    return Color{
        base, base, static_cast<unsigned char>(base + 4), 255,
    };
}

