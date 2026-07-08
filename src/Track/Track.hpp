/*
** EPITECH PROJECT, 2026
** racer
** File description:
** track layout generation and waypoint queries
*/

#ifndef TRACK_HPP_
#define TRACK_HPP_

#include <vector>

#include "raylib.h"

#include "Track/TrackDef.hpp"

namespace racer {

class Track {
public:
    struct Progress {
        int segmentIndex = 0;
        float t = 0.0f;
        float lateralOffset = 0.0f;
    };

    static Track make(const TrackDef& def);
    static const std::vector<TrackDef>& presets();

    Vector3 startPosition(int laneIndex, int laneCount) const;
    float startHeading() const;

    Progress projectPosition(Vector3 pos) const;
    float cumulativeDistance(const Progress& p) const;
    float totalLength() const;
    Vector2 pointAtDistance(float distance) const;

    const std::vector<Vector2>& waypoints() const { return waypoints_; }
    float width() const { return width_; }
    SurfaceStyle style() const { return style_; }

private:
    struct SegmentSample {
        Progress progress;
        float distSq;
    };

    void recomputeLengths();

    static float segmentLength(Vector2 a, Vector2 b);
    static void appendEastStraight(
        const TrackDef& def, std::vector<Vector2>& waypoints);
    static void appendNorthCurve(
        const TrackDef& def, std::vector<Vector2>& waypoints);
    static void appendWestStraight(
        const TrackDef& def, std::vector<Vector2>& waypoints);
    static void appendSouthCurve(
        const TrackDef& def, std::vector<Vector2>& waypoints);
    SegmentSample sampleSegment(Vector3 pos, std::size_t index) const;

    std::vector<Vector2> waypoints_;
    std::vector<float> cumulativeLengths_;
    float totalLength_ = 0.0f;
    float width_ = 12.0f;
    SurfaceStyle style_ = SurfaceStyle::PROPRE;
};

} // namespace racer

#endif /* !TRACK_HPP_ */
