/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Track skid mark overlay
*/

#ifndef TRACK_SKID_MARKS_HPP_
#define TRACK_SKID_MARKS_HPP_

#include "raylib.h"
#include "Render/Track/TrackInstanceTypes.hpp"

namespace racer {

class TrackRenderer;

struct TrackSkidMarks {
    static void queue(
        TrackRenderer &renderer, Vector3 pos, Vector3 dir,
        float width, float strength);
    static void flush(TrackRenderer &renderer);
    static void drawOne(
        const TrackSkidMarkCmd &cmd, Vector2 origin, float worldSize);
};

} // namespace racer

#endif /* !TRACK_SKID_MARKS_HPP_ */
