/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Track layout definition and surface style
*/

#ifndef TRACK_DEF_HPP_
#define TRACK_DEF_HPP_

#include <string>

namespace racer {

enum class SurfaceStyle { PROPRE, ABIMEE };

struct TrackDef {
    std::string name;
    std::string description;
    float straightLength = 90.0f;
    float radius = 16.0f;
    float width = 11.0f;
    float chicaneAmpEast = 9.0f;
    float chicaneAmpWest = 6.0f;
    float chicaneFreqWest = 2.0f;
    SurfaceStyle surfaceStyle = SurfaceStyle::PROPRE;
};

} // namespace racer

#endif /* !TRACK_DEF_HPP_ */
