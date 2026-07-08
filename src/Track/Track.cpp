/*
** EPITECH PROJECT, 2026
** racer
** File description:
** track layout generation and waypoint queries
*/

#include "Track/Track.hpp"

#include <cmath>

namespace racer {

namespace {

constexpr int kCurveSegments = 20;
constexpr int kStraightSegments = 24;
constexpr float kHalf = 0.5f;
constexpr float kChicaneCycles = 2.0f;
constexpr float kMinSegmentLength = 1e-6f;
constexpr float kLaneSpreadFactor = 0.6f;
constexpr float kLaneCenter = 0.5f;
constexpr float kLaneBackSpacing = 4.0f;
constexpr float kMaxDistSq = 1e30f;


float wrapTrackDistance(float distance, float totalLength)
{
    float d = std::fmod(distance, totalLength);
    if (d < 0.0f)
        d += totalLength;
    return d;
}

float segmentEndAt(
    std::size_t index, std::size_t count,
    const std::vector<float>& cumulativeLengths, float totalLength)
{
    if (index + 1 < count)
        return cumulativeLengths[index + 1];
    return totalLength;
}

bool containsDistance(
    float distance, std::size_t index, std::size_t count,
    const std::vector<float>& cumulativeLengths, float totalLength)
{
    float segEnd = segmentEndAt(index, count, cumulativeLengths, totalLength);
    return distance <= segEnd || index == count - 1;
}

Vector2 interpolateWaypoints(Vector2 a, Vector2 b, float t)
{
    return Vector2{
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
    };
}

Vector2 pointOnSegmentAtDistance(
    float distance, std::size_t index, std::size_t count,
    const std::vector<Vector2>& waypoints,
    const std::vector<float>& cumulativeLengths, float totalLength)
{
    float segStart = cumulativeLengths[index];
    float segEnd = segmentEndAt(index, count, cumulativeLengths, totalLength);
    float segLen = segEnd - segStart;
    float t = 0.0f;
    if (segLen > kMinSegmentLength)
        t = (distance - segStart) / segLen;
    Vector2 a = waypoints[index];
    Vector2 b = waypoints[(index + 1) % count];
    return interpolateWaypoints(a, b, t);
}

} // namespace

Track Track::make(const TrackDef& def)
{
    Track t;
    t.width_ = def.width;
    t.style_ = def.surfaceStyle;

    appendEastStraight(def, t.waypoints_);
    appendNorthCurve(def, t.waypoints_);
    appendWestStraight(def, t.waypoints_);
    appendSouthCurve(def, t.waypoints_);
    t.recomputeLengths();
    return t;
}

void Track::recomputeLengths()
{
    std::size_t n = waypoints_.size();
    cumulativeLengths_.assign(n, 0.0f);
    float acc = 0.0f;
    for (std::size_t i = 0; i < n; ++i) {
        cumulativeLengths_[i] = acc;
        Vector2 a = waypoints_[i];
        Vector2 b = waypoints_[(i + 1) % n];
        acc += segmentLength(a, b);
    }
    totalLength_ = acc;
}

float Track::segmentLength(Vector2 a, Vector2 b)
{
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    return std::sqrt(dx * dx + dy * dy);
}

Vector3 Track::startPosition(int laneIndex, int laneCount) const
{
    Vector2 a = waypoints_[0];
    Vector2 b = waypoints_[1];
    Vector2 dir{b.x - a.x, b.y - a.y};
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    dir.x /= len;
    dir.y /= len;
    Vector2 perp{-dir.y, dir.x};

    float spread = width_ * kLaneSpreadFactor;
    float lane01 = kLaneCenter;
    if (laneCount > 1) {
        lane01 = static_cast<float>(laneIndex)
            / static_cast<float>(laneCount - 1);
    }
    float offset = (lane01 - kLaneCenter) * spread;
    float back = static_cast<float>(laneIndex) * kLaneBackSpacing;

    return Vector3{
        a.x + perp.x * offset - dir.x * back,
        0.0f,
        a.y + perp.y * offset - dir.y * back,
    };
}

float Track::startHeading() const
{
    Vector2 a = waypoints_[0];
    Vector2 b = waypoints_[1];
    return std::atan2(b.x - a.x, b.y - a.y);
}

Track::SegmentSample Track::sampleSegment(Vector3 pos, std::size_t index) const
{
    SegmentSample sample{};
    std::size_t n = waypoints_.size();
    Vector2 a = waypoints_[index];
    Vector2 b = waypoints_[(index + 1) % n];
    Vector2 ab{b.x - a.x, b.y - a.y};
    float segLenSq = ab.x * ab.x + ab.y * ab.y;
    Vector2 ap{pos.x - a.x, pos.z - a.y};

    float t = 0.0f;
    if (segLenSq > kMinSegmentLength)
        t = (ap.x * ab.x + ap.y * ab.y) / segLenSq;
    float tc = t;
    if (tc < 0.0f)
        tc = 0.0f;
    else if (tc > 1.0f)
        tc = 1.0f;

    Vector2 proj{a.x + ab.x * tc, a.y + ab.y * tc};
    float dx = pos.x - proj.x;
    float dz = pos.z - proj.y;
    sample.distSq = dx * dx + dz * dz;
    sample.progress.segmentIndex = static_cast<int>(index);
    sample.progress.t = tc;

    float cross = ab.x * ap.y - ab.y * ap.x;
    float segLen = std::sqrt(segLenSq);
    if (segLen > kMinSegmentLength)
        sample.progress.lateralOffset = cross / segLen;
    return sample;
}

Track::Progress Track::projectPosition(Vector3 pos) const
{
    Progress best;
    float bestDistSq = kMaxDistSq;
    std::size_t n = waypoints_.size();
    for (std::size_t i = 0; i < n; ++i) {
        SegmentSample sample = sampleSegment(pos, i);
        if (sample.distSq < bestDistSq) {
            bestDistSq = sample.distSq;
            best = sample.progress;
        }
    }
    return best;
}

float Track::cumulativeDistance(const Progress& p) const
{
    std::size_t n = waypoints_.size();
    std::size_t idx = static_cast<std::size_t>(p.segmentIndex);
    Vector2 a = waypoints_[idx];
    Vector2 b = waypoints_[(idx + 1) % n];
    float segLen = segmentLength(a, b);
    return cumulativeLengths_[idx] + p.t * segLen;
}

float Track::totalLength() const
{
    return totalLength_;
}

Vector2 Track::pointAtDistance(float distance) const
{
    std::size_t n = waypoints_.size();
    float d = wrapTrackDistance(distance, totalLength_);
    Vector2 result = waypoints_[0];
    for (std::size_t i = 0; i < n; ++i) {
        if (!containsDistance(d, i, n, cumulativeLengths_, totalLength_))
            continue;
        result = pointOnSegmentAtDistance(
            d, i, n, waypoints_, cumulativeLengths_, totalLength_);
        break;
    }
    return result;
}

} // namespace racer
