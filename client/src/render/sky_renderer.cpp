#include "client/render/sky_renderer.h"

#include <cmath>

namespace client {

namespace {

Color LerpColor(Color a, Color b, float t) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return Color{
        static_cast<unsigned char>(a.r + (static_cast<float>(b.r) - a.r) * t),
        static_cast<unsigned char>(a.g + (static_cast<float>(b.g) - a.g) * t),
        static_cast<unsigned char>(a.b + (static_cast<float>(b.b) - a.b) * t),
        static_cast<unsigned char>(a.a + (static_cast<float>(b.a) - a.a) * t),
    };
}

constexpr Color kNightSky{10, 10, 35, 255};
constexpr Color kDaySky{135, 206, 235, 255}; // SKYBLUE
constexpr Color kNightTint{60, 60, 90, 255};
constexpr Color kDayTint{255, 255, 255, 255};

} // namespace

DayNightState ComputeDayNight(float timeOfDay01) {
    float angle = timeOfDay01 * 2.0f * PI;
    float brightness = (std::sin(angle - PI / 2.0f) + 1.0f) * 0.5f; // 0 a minuit, 1 a midi

    DayNightState state;
    state.skyColor = LerpColor(kNightSky, kDaySky, brightness);
    state.groundTint = LerpColor(kNightTint, kDayTint, brightness);
    return state;
}

} // namespace client
