/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Track decor instance types
*/

#ifndef TRACK_INSTANCE_TYPES_HPP_
#define TRACK_INSTANCE_TYPES_HPP_

#include <vector>

#include "raylib.h"

namespace racer {

struct TrackPropInstance {
    Vector3 position;
    float heightScale;
    int type;
    Color color;
};

struct TrackCloudInstance {
    Vector3 basePosition;
    float driftSpeed;
    float scale;
    std::vector<Vector3> puffOffsets;
    std::vector<float> puffScales;
};

struct TrackSpectatorInstance {
    Vector3 position;
    Color shirtColor;
    float jumpPhase;
    float jumpSpeed;
};

struct TrackGrandstandInstance {
    Vector3 origin;
    Vector3 along;
    Vector3 outward;
    float length;
    std::vector<TrackSpectatorInstance> spectators;
};

struct TrackNpcInstance {
    Vector3 position;
    float heading;
    Color shirtColor;
    Color flagColor;
    float animPhase;
};

struct TrackTireStackInstance {
    Vector3 position;
    int tiers;
};

struct TrackPennantInstance {
    Vector3 base;
    Vector3 top;
    Color color;
    float phase;
};

struct TrackPotholeInstance {
    Vector3 position;
    float radius;
};

struct TrackCrackInstance {
    Vector3 center;
    Vector3 tangent;
    float length;
};

struct TrackSkidMarkCmd {
    Vector3 position;
    Vector3 direction;
    float width;
    float strength;
};

struct TrackLampInstance {
    Vector3 base;
    Vector3 top;
    Color headColor;
    bool lit;
};

struct TrackArchInstance {
    Vector3 leftBase;
    Vector3 rightBase;
    Color colorA;
    Color colorB;
};

} // namespace racer

#endif /* !TRACK_INSTANCE_TYPES_HPP_ */
