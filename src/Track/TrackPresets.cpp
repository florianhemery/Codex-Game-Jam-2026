/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Track layout preset definitions
*/

#include "Track/Track.hpp"

namespace racer {

constexpr float kAnneauStraight = 370.0f;
constexpr float kAnneauRadius = 68.0f;
constexpr float kAnneauWidth = 30.0f;
constexpr float kAnneauChicaneEast = 0.0f;
constexpr float kAnneauChicaneWest = 0.0f;
constexpr float kAnneauChicaneFreq = 1.0f;

constexpr float kSinueuxStraight = 250.0f;
constexpr float kSinueuxRadius = 44.0f;
constexpr float kSinueuxWidth = 26.0f;
constexpr float kSinueuxChicaneEast = 18.0f;
constexpr float kSinueuxChicaneWest = 12.0f;
constexpr float kSinueuxChicaneFreq = 2.0f;

constexpr float kTechniqueStraight = 156.0f;
constexpr float kTechniqueRadius = 32.0f;
constexpr float kTechniqueWidth = 24.0f;
constexpr float kTechniqueChicaneEast = 16.0f;
constexpr float kTechniqueChicaneWest = 18.0f;
constexpr float kTechniqueChicaneFreq = 3.0f;

constexpr float kAbimeeStraight = 196.0f;
constexpr float kAbimeeRadius = 38.0f;
constexpr float kAbimeeWidth = 24.0f;
constexpr float kAbimeeChicaneEast = 10.0f;
constexpr float kAbimeeChicaneWest = 14.0f;
constexpr float kAbimeeChicaneFreq = 2.0f;

TrackDef makeAnneauVitessePreset()
{
    return {
        "Anneau Vitesse",
        "Grand ovale rapide avec longues lignes droites",
        kAnneauStraight, kAnneauRadius, kAnneauWidth,
        kAnneauChicaneEast, kAnneauChicaneWest, kAnneauChicaneFreq,
        SurfaceStyle::PROPRE,
    };
}

TrackDef makeCircuitSinueuxPreset()
{
    return {
        "Circuit Sinueux",
        "Virages serres et chicanes prononcees",
        kSinueuxStraight, kSinueuxRadius, kSinueuxWidth,
        kSinueuxChicaneEast, kSinueuxChicaneWest, kSinueuxChicaneFreq,
        SurfaceStyle::PROPRE,
    };
}

TrackDef makeCircuitTechniquePreset()
{
    return {
        "Circuit Technique",
        "Tres serre avec chicanes techniques",
        kTechniqueStraight, kTechniqueRadius, kTechniqueWidth,
        kTechniqueChicaneEast, kTechniqueChicaneWest, kTechniqueChicaneFreq,
        SurfaceStyle::PROPRE,
    };
}

TrackDef makeRouteAbimeePreset()
{
    return {
        "Route Abimee",
        "Chaussee delavee et creux accidentes",
        kAbimeeStraight, kAbimeeRadius, kAbimeeWidth,
        kAbimeeChicaneEast, kAbimeeChicaneWest, kAbimeeChicaneFreq,
        SurfaceStyle::ABIMEE,
    };
}

std::vector<TrackDef> buildTrackPresets()
{
    return {
        makeAnneauVitessePreset(),
        makeCircuitSinueuxPreset(),
        makeCircuitTechniquePreset(),
        makeRouteAbimeePreset(),
    };
}

const std::vector<TrackDef>& Track::presets()
{
    static const std::vector<TrackDef> presets = buildTrackPresets();
    return presets;
}

} // namespace racer
