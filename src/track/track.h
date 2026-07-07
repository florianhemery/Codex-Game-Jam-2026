#pragma once

#include <string>
#include <vector>

#include "raylib.h"

namespace racer {

enum class SurfaceStyle { Propre, Abimee };

// Parametres d'un circuit "stade" (2 lignes droites + 2 virages en demi-cercle,
// topologie toujours garantie fermee quels que soient les parametres --
// les chicanes sont des perturbations laterales qui reviennent a zero aux
// extremites de la ligne droite, donc ne cassent jamais la fermeture).
struct TrackDef {
    std::string name;
    std::string description;
    float straightLength = 90.0f;
    float radius = 16.0f;
    float width = 11.0f;
    float chicaneAmpEast = 9.0f;   // 0 = pas de chicane sur la ligne est
    float chicaneAmpWest = 6.0f;
    float chicaneFreqWest = 2.0f;  // 1 = un seul virage en S, 2 = deux (esses)
    SurfaceStyle surfaceStyle = SurfaceStyle::Propre;
};

// Piste = boucle fermee de waypoints (plan XZ, Y=0). Segment i relie
// waypoints[i] a waypoints[(i+1)%N]. Pas de hauteur/relief en V1.
class Track {
public:
    struct Progress {
        int segmentIndex = 0;
        float t = 0.0f;             // 0..1 le long du segment
        float lateralOffset = 0.0f; // signe ; hors piste si |offset| > width/2
    };

    static Track Make(const TrackDef& def);
    static const std::vector<TrackDef>& Presets();

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
    SurfaceStyle Style() const { return style_; } // etat de la chaussee, copie depuis TrackDef

private:
    void RecomputeLengths();

    std::vector<Vector2> waypoints_;
    std::vector<float> cumulativeLengths_; // taille = waypoints_.size(), longueur cumulee AVANT le segment i
    float totalLength_ = 0.0f;
    float width_ = 12.0f;
    SurfaceStyle style_ = SurfaceStyle::Propre;
};

} // namespace racer
