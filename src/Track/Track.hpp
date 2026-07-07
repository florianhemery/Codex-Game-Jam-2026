/*
** EPITECH PROJECT, 2026
** racer
** File description:
** track layout generation and waypoint queries
*/

#ifndef TRACK_HPP_
#define TRACK_HPP_

#include <string>
#include <vector>

#include "raylib.h"

namespace racer {

enum class SurfaceStyle { Propre, Abimee };

struct TrackDef {
    std::string name;
    std::string description;
    float straightLength = 90.0f;
    float radius = 16.0f;
    float width = 11.0f;
    float chicaneAmpEast = 9.0f;
    float chicaneAmpWest = 6.0f;
    float chicaneFreqWest = 2.0f;
    SurfaceStyle surfaceStyle = SurfaceStyle::Propre;
};

class Track {
public:
    struct Progress {
        int segmentIndex = 0;
        float t = 0.0f;
        float lateralOffset = 0.0f;
    };

    static Track Make(const TrackDef& def);
    static const std::vector<TrackDef>& Presets();

    Vector3 StartPosition(int laneIndex, int laneCount) const;
    float StartHeading() const;

    Progress ProjectPosition(Vector3 pos) const;
    float CumulativeDistance(const Progress& p) const;
    float TotalLength() const;
    Vector2 PointAtDistance(float distance) const;

    const std::vector<Vector2>& Waypoints() const { return waypoints_; }
    float Width() const { return width_; }
    SurfaceStyle Style() const { return style_; }

private:
    struct SegmentSample {
        Progress progress;
        float distSq;
    };

    void RecomputeLengths();

    static float SegmentLength(Vector2 a, Vector2 b);
    static void AppendEastStraight(
        const TrackDef& def, std::vector<Vector2>& waypoints);
    static void AppendNorthCurve(
        const TrackDef& def, std::vector<Vector2>& waypoints);
    static void AppendWestStraight(
        const TrackDef& def, std::vector<Vector2>& waypoints);
    static void AppendSouthCurve(
        const TrackDef& def, std::vector<Vector2>& waypoints);
    SegmentSample SampleSegment(Vector3 pos, std::size_t index) const;

    std::vector<Vector2> waypoints_;
    std::vector<float> cumulativeLengths_;
    float totalLength_ = 0.0f;
    float width_ = 12.0f;
    SurfaceStyle style_ = SurfaceStyle::Propre;
};

} // namespace racer

#endif /* !TRACK_HPP_ */
