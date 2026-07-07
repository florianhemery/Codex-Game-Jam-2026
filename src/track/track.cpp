#include "track/track.h"

#include <cmath>

namespace racer {

namespace {
float Length(Vector2 a, Vector2 b) {
    return std::sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
}
} // namespace

Track Track::Make(const TrackDef& def) {
    Track t;
    t.width_ = def.width;

    constexpr int kCurveSegments = 20;
    constexpr int kStraightSegments = 24; // subdivise pour que les chicanes soient lisses

    std::vector<Vector2>& wp = t.waypoints_;

    // Ligne droite "est" (x=+radius) avec une chicane (un aller-retour lateral
    // complet, nul aux deux extremites -- ne casse jamais la fermeture de la boucle).
    for (int i = 0; i <= kStraightSegments; ++i) {
        float lt = static_cast<float>(i) / kStraightSegments;
        float z = -def.straightLength / 2.0f + lt * def.straightLength;
        float chicane = def.chicaneAmpEast * std::sin(2.0f * PI * lt);
        wp.push_back({def.radius + chicane, z});
    }

    // Virage nord (centre (0, +straight/2)), angle 0 -> pi.
    for (int i = 1; i < kCurveSegments; ++i) {
        float a = (static_cast<float>(i) / kCurveSegments) * PI;
        wp.push_back({def.radius * std::cos(a), def.straightLength / 2.0f + def.radius * std::sin(a)});
    }

    // Ligne droite "ouest" (x=-radius), chicane independante (frequence/amplitude
    // propres) pour varier la sensation par rapport au cote est.
    for (int i = 0; i <= kStraightSegments; ++i) {
        float lt = static_cast<float>(i) / kStraightSegments;
        float z = def.straightLength / 2.0f - lt * def.straightLength;
        float chicane = def.chicaneAmpWest * std::sin(def.chicaneFreqWest * 2.0f * PI * lt);
        wp.push_back({-def.radius + chicane, z});
    }

    // Virage sud (centre (0, -straight/2)), angle pi -> 2pi.
    for (int i = 1; i < kCurveSegments; ++i) {
        float a = PI + (static_cast<float>(i) / kCurveSegments) * PI;
        wp.push_back({def.radius * std::cos(a), -def.straightLength / 2.0f + def.radius * std::sin(a)});
    }

    t.RecomputeLengths();
    return t;
}

const std::vector<TrackDef>& Track::Presets() {
    static const std::vector<TrackDef> presets = {
        {
            "Anneau Vitesse",
            "Grand ovale rapide, longues lignes droites, pas de chicane",
            120.0f, 22.0f, 13.0f, 0.0f, 0.0f, 1.0f,
        },
        {
            "Circuit Sinueux",
            "Un peu de tout : virages serres et deux chicanes",
            90.0f, 16.0f, 11.0f, 9.0f, 6.0f, 2.0f,
        },
        {
            "Circuit Technique",
            "Court, tres serre, chicanes prononcees -- ideal pour driver",
            55.0f, 11.0f, 10.0f, 8.0f, 9.0f, 3.0f,
        },
    };
    return presets;
}

void Track::RecomputeLengths() {
    size_t n = waypoints_.size();
    cumulativeLengths_.assign(n, 0.0f);
    float acc = 0.0f;
    for (size_t i = 0; i < n; ++i) {
        cumulativeLengths_[i] = acc;
        Vector2 a = waypoints_[i];
        Vector2 b = waypoints_[(i + 1) % n];
        acc += Length(a, b);
    }
    totalLength_ = acc;
}

Vector3 Track::StartPosition(int laneIndex, int laneCount) const {
    Vector2 a = waypoints_[0];
    Vector2 b = waypoints_[1];
    Vector2 dir{b.x - a.x, b.y - a.y};
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    dir.x /= len;
    dir.y /= len;
    Vector2 perp{-dir.y, dir.x};

    float spread = width_ * 0.6f;
    float lane01 = (laneCount <= 1) ? 0.5f : static_cast<float>(laneIndex) / static_cast<float>(laneCount - 1);
    float offset = (lane01 - 0.5f) * spread;

    // Recule chaque voiture le long de la grille de depart pour ne pas se superposer.
    float back = static_cast<float>(laneIndex) * 4.0f;

    return Vector3{
        a.x + perp.x * offset - dir.x * back,
        0.0f,
        a.y + perp.y * offset - dir.y * back,
    };
}

float Track::StartHeading() const {
    Vector2 a = waypoints_[0];
    Vector2 b = waypoints_[1];
    return std::atan2(b.x - a.x, b.y - a.y);
}

Track::Progress Track::ProjectPosition(Vector3 pos) const {
    Progress best;
    float bestDistSq = 1e30f;
    size_t n = waypoints_.size();

    for (size_t i = 0; i < n; ++i) {
        Vector2 a = waypoints_[i];
        Vector2 b = waypoints_[(i + 1) % n];
        Vector2 ab{b.x - a.x, b.y - a.y};
        float segLenSq = ab.x * ab.x + ab.y * ab.y;
        Vector2 ap{pos.x - a.x, pos.z - a.y};

        float t = segLenSq > 1e-6f ? (ap.x * ab.x + ap.y * ab.y) / segLenSq : 0.0f;
        float tc = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
        Vector2 proj{a.x + ab.x * tc, a.y + ab.y * tc};
        float dx = pos.x - proj.x;
        float dz = pos.z - proj.y;
        float distSq = dx * dx + dz * dz;

        if (distSq < bestDistSq) {
            bestDistSq = distSq;
            best.segmentIndex = static_cast<int>(i);
            best.t = tc;
            // offset lateral signe : produit vectoriel 2D (ab x ap)
            float cross = ab.x * ap.y - ab.y * ap.x;
            float segLen = std::sqrt(segLenSq);
            best.lateralOffset = segLen > 1e-6f ? cross / segLen : 0.0f;
        }
    }

    return best;
}

float Track::CumulativeDistance(const Progress& p) const {
    size_t n = waypoints_.size();
    Vector2 a = waypoints_[static_cast<size_t>(p.segmentIndex)];
    Vector2 b = waypoints_[(static_cast<size_t>(p.segmentIndex) + 1) % n];
    float segLen = Length(a, b);
    return cumulativeLengths_[static_cast<size_t>(p.segmentIndex)] + p.t * segLen;
}

float Track::TotalLength() const {
    return totalLength_;
}

Vector2 Track::PointAtDistance(float distance) const {
    size_t n = waypoints_.size();
    float d = std::fmod(distance, totalLength_);
    if (d < 0.0f) d += totalLength_;

    for (size_t i = 0; i < n; ++i) {
        float segStart = cumulativeLengths_[i];
        float segEnd = (i + 1 < n) ? cumulativeLengths_[i + 1] : totalLength_;
        if (d <= segEnd || i == n - 1) {
            float segLen = segEnd - segStart;
            float t = segLen > 1e-6f ? (d - segStart) / segLen : 0.0f;
            Vector2 a = waypoints_[i];
            Vector2 b = waypoints_[(i + 1) % n];
            return Vector2{a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t};
        }
    }
    return waypoints_[0];
}

} // namespace racer
