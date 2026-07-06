#pragma once

#include <vector>

#include "raylib.h"

namespace racer {

// Piste = boucle fermee de waypoints (plan XZ, Y=0). Segment i relie
// waypoints[i] a waypoints[(i+1)%N]. Pas de hauteur/relief en V1.
class Track {
public:
    struct Progress {
        int segmentIndex = 0;
        float t = 0.0f;             // 0..1 le long du segment
        float lateralOffset = 0.0f; // signe ; hors piste si |offset| > width/2
    };

    static Track MakeStadiumTrack();

    Vector3 StartPosition(int laneIndex, int laneCount) const;
    float StartHeading() const;

    Progress ProjectPosition(Vector3 pos) const;
    float CumulativeDistance(const Progress& p) const;
    float TotalLength() const;

    // Point sur la ligne centrale a une distance cumulee donnee (repliee
    // modulo la longueur totale) -- utilise pour le lookahead de l'IA.
    Vector2 PointAtDistance(float distance) const;

    const std::vector<Vector2>& Waypoints() const { return waypoints_; }
    float Width() const { return width_; }

private:
    void RecomputeLengths();

    std::vector<Vector2> waypoints_;
    std::vector<float> cumulativeLengths_; // taille = waypoints_.size(), longueur cumulee AVANT le segment i
    float totalLength_ = 0.0f;
    float width_ = 12.0f;
};

} // namespace racer
