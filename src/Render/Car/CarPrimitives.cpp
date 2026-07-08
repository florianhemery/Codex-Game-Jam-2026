/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Curved car primitives with authored normals
*/

#include "Render/Car/CarPrimitives.hpp"

#include "raymath.h"
#include "rlgl.h"

namespace racer {

void DrawCylinderExLit(
    Vector3 startPos, Vector3 endPos,
    float startRadius, float endRadius, int sides, Color color)
{
    if (sides < 3)
        sides = 3;

    Vector3 direction = {
        endPos.x - startPos.x,
        endPos.y - startPos.y,
        endPos.z - startPos.z,
    };
    if (direction.x == 0.0f && direction.y == 0.0f && direction.z == 0.0f)
        return;

    Vector3 axis = Vector3Normalize(direction);
    Vector3 b1 = Vector3Normalize(Vector3Perpendicular(direction));
    Vector3 b2 = Vector3Normalize(Vector3CrossProduct(b1, direction));

    const float baseAngle = (2.0f * PI) / static_cast<float>(sides);

    rlBegin(RL_TRIANGLES);
    rlColor4ub(color.r, color.g, color.b, color.a);

    for (int i = 0; i < sides; i++) {
        const float s1 = sinf(baseAngle * static_cast<float>(i + 0));
        const float c1 = cosf(baseAngle * static_cast<float>(i + 0));
        const float s2 = sinf(baseAngle * static_cast<float>(i + 1));
        const float c2 = cosf(baseAngle * static_cast<float>(i + 1));

        const Vector3 n1{
            s1 * b1.x + c1 * b2.x, s1 * b1.y + c1 * b2.y,
            s1 * b1.z + c1 * b2.z};
        const Vector3 n2{
            s2 * b1.x + c2 * b2.x, s2 * b1.y + c2 * b2.y,
            s2 * b1.z + c2 * b2.z};

        const Vector3 w1{
            startPos.x + n1.x * startRadius, startPos.y + n1.y * startRadius,
            startPos.z + n1.z * startRadius};
        const Vector3 w2{
            startPos.x + n2.x * startRadius, startPos.y + n2.y * startRadius,
            startPos.z + n2.z * startRadius};
        const Vector3 w3{
            endPos.x + n1.x * endRadius, endPos.y + n1.y * endRadius,
            endPos.z + n1.z * endRadius};
        const Vector3 w4{
            endPos.x + n2.x * endRadius, endPos.y + n2.y * endRadius,
            endPos.z + n2.z * endRadius};

        if (startRadius > 0.0f) {
            rlNormal3f(-axis.x, -axis.y, -axis.z);
            rlVertex3f(startPos.x, startPos.y, startPos.z);
            rlVertex3f(w2.x, w2.y, w2.z);
            rlVertex3f(w1.x, w1.y, w1.z);
        }

        rlNormal3f(n1.x, n1.y, n1.z);
        rlVertex3f(w1.x, w1.y, w1.z);
        rlNormal3f(n2.x, n2.y, n2.z);
        rlVertex3f(w2.x, w2.y, w2.z);
        rlNormal3f(n1.x, n1.y, n1.z);
        rlVertex3f(w3.x, w3.y, w3.z);

        rlNormal3f(n2.x, n2.y, n2.z);
        rlVertex3f(w2.x, w2.y, w2.z);
        rlVertex3f(w4.x, w4.y, w4.z);
        rlNormal3f(n1.x, n1.y, n1.z);
        rlVertex3f(w3.x, w3.y, w3.z);

        if (endRadius > 0.0f) {
            rlNormal3f(axis.x, axis.y, axis.z);
            rlVertex3f(endPos.x, endPos.y, endPos.z);
            rlVertex3f(w3.x, w3.y, w3.z);
            rlVertex3f(w4.x, w4.y, w4.z);
        }
    }

    rlEnd();
}

void DrawSphereLit(
    Vector3 centerPos, float radius, int rings, int slices, Color color)
{
    rlPushMatrix();
    rlTranslatef(centerPos.x, centerPos.y, centerPos.z);
    rlScalef(radius, radius, radius);

    rlBegin(RL_TRIANGLES);
    rlColor4ub(color.r, color.g, color.b, color.a);

    const float ringAngle = DEG2RAD * (180.0f / static_cast<float>(rings + 1));
    const float sliceAngle = DEG2RAD * (360.0f / static_cast<float>(slices));

    const float cosRing = cosf(ringAngle);
    const float sinRing = sinf(ringAngle);
    const float cosSlice = cosf(sliceAngle);
    const float sinSlice = sinf(sliceAngle);

    Vector3 vertices[4] = {};
    vertices[2] = Vector3{0.0f, 1.0f, 0.0f};
    vertices[3] = Vector3{sinRing, cosRing, 0.0f};

    for (int i = 0; i < rings + 1; i++) {
        for (int j = 0; j < slices; j++) {
            vertices[0] = vertices[2];
            vertices[1] = vertices[3];
            vertices[2] = Vector3{
                cosSlice * vertices[2].x - sinSlice * vertices[2].z,
                vertices[2].y,
                sinSlice * vertices[2].x + cosSlice * vertices[2].z};
            vertices[3] = Vector3{
                cosSlice * vertices[3].x - sinSlice * vertices[3].z,
                vertices[3].y,
                sinSlice * vertices[3].x + cosSlice * vertices[3].z};

            rlNormal3f(vertices[0].x, vertices[0].y, vertices[0].z);
            rlVertex3f(vertices[0].x, vertices[0].y, vertices[0].z);
            rlNormal3f(vertices[3].x, vertices[3].y, vertices[3].z);
            rlVertex3f(vertices[3].x, vertices[3].y, vertices[3].z);
            rlNormal3f(vertices[1].x, vertices[1].y, vertices[1].z);
            rlVertex3f(vertices[1].x, vertices[1].y, vertices[1].z);

            rlNormal3f(vertices[0].x, vertices[0].y, vertices[0].z);
            rlVertex3f(vertices[0].x, vertices[0].y, vertices[0].z);
            rlNormal3f(vertices[2].x, vertices[2].y, vertices[2].z);
            rlVertex3f(vertices[2].x, vertices[2].y, vertices[2].z);
            rlNormal3f(vertices[3].x, vertices[3].y, vertices[3].z);
            rlVertex3f(vertices[3].x, vertices[3].y, vertices[3].z);
        }

        vertices[2] = vertices[3];
        vertices[3] = Vector3{
            cosRing * vertices[3].x + sinRing * vertices[3].y,
            -sinRing * vertices[3].x + cosRing * vertices[3].y,
            vertices[3].z};
    }

    rlEnd();
    rlPopMatrix();
}

} // namespace racer
