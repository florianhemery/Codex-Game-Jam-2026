/*
** EPITECH PROJECT, 2026
** racer
** File description:
** DrawCube/DrawSphere wrappers for lit-shader car pass
*/

#ifndef CAR_DRAW_HPP_
#define CAR_DRAW_HPP_

#include "raylib.h"

namespace racer {
namespace carDraw {

void beginPass(Shader lit, Color fallbackTint);
void endPass(Shader lit);
void setColor(Color color);
bool isActive();

inline void cube(
    Vector3 position, float width, float height, float length, Color color)
{
    if (isActive()) {
        setColor(color);
        DrawCube(position, width, height, length, WHITE);
    } else {
        DrawCube(position, width, height, length, color);
    }
}

inline void sphere(Vector3 centerPos, float radius, Color color)
{
    if (isActive()) {
        setColor(color);
        DrawSphere(centerPos, radius, WHITE);
    } else {
        DrawSphere(centerPos, radius, color);
    }
}

inline void cylinder(
    Vector3 position, float radiusTop, float radiusBottom, float height,
    int slices, Color color)
{
    if (isActive()) {
        setColor(color);
        DrawCylinder(position, radiusTop, radiusBottom, height, slices, WHITE);
    } else {
        DrawCylinder(position, radiusTop, radiusBottom, height, slices, color);
    }
}

inline void cylinderEx(
    Vector3 startPos, Vector3 endPos, float startRadius, float endRadius,
    int sides, Color color)
{
    if (isActive()) {
        setColor(color);
        DrawCylinderEx(
            startPos, endPos, startRadius, endRadius, sides, WHITE);
    } else {
        DrawCylinderEx(
            startPos, endPos, startRadius, endRadius, sides, color);
    }
}

} // namespace carDraw
} // namespace racer

#endif /* !CAR_DRAW_HPP_ */
