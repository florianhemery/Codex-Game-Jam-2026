/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Track waypoint ring append helpers
*/

#include "Track/Track.hpp"

#include <cmath>

namespace racer {

namespace {
constexpr int kCurveSegments = 20;
constexpr int kStraightSegments = 24;
constexpr float kHalf = 0.5f;
constexpr float kChicaneCycles = 2.0f;
} // namespace

void Track::appendEastStraight(
    const TrackDef& def, std::vector<Vector2>& waypoints)
{
    for (int i = 0; i <= kStraightSegments; ++i) {
        float iF = static_cast<float>(i);
        float segF = static_cast<float>(kStraightSegments);
        float lt = iF / segF;
        float z = -def.straightLength * kHalf + lt * def.straightLength;
        float chicane = def.chicaneAmpEast * std::sin(kChicaneCycles * PI * lt);
        waypoints.push_back({def.radius + chicane, z});
    }
}

void Track::appendNorthCurve(
    const TrackDef& def, std::vector<Vector2>& waypoints)
{
    for (int i = 1; i < kCurveSegments; ++i) {
        float iF = static_cast<float>(i);
        float segF = static_cast<float>(kCurveSegments);
        float a = (iF / segF) * PI;
        float x = def.radius * std::cos(a);
        float y = def.straightLength * kHalf + def.radius * std::sin(a);
        waypoints.push_back({x, y});
    }
}

void Track::appendWestStraight(
    const TrackDef& def, std::vector<Vector2>& waypoints)
{
    for (int i = 0; i <= kStraightSegments; ++i) {
        float iF = static_cast<float>(i);
        float segF = static_cast<float>(kStraightSegments);
        float lt = iF / segF;
        float z = def.straightLength * kHalf - lt * def.straightLength;
        float phase = def.chicaneFreqWest * kChicaneCycles * PI * lt;
        float chicane = def.chicaneAmpWest * std::sin(phase);
        waypoints.push_back({-def.radius + chicane, z});
    }
}

void Track::appendSouthCurve(
    const TrackDef& def, std::vector<Vector2>& waypoints)
{
    for (int i = 1; i < kCurveSegments; ++i) {
        float iF = static_cast<float>(i);
        float segF = static_cast<float>(kCurveSegments);
        float a = PI + (iF / segF) * PI;
        float x = def.radius * std::cos(a);
        float y = -def.straightLength * kHalf + def.radius * std::sin(a);
        waypoints.push_back({x, y});
    }
}

} // namespace racer
