/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Curved car primitives with authored normals
*/

#ifndef CAR_PRIMITIVES_HPP_
#define CAR_PRIMITIVES_HPP_

#include "raylib.h"

namespace racer {

// Meme geometrie que raylib::DrawCylinderEx, mais emet une normale radiale
// par sommet : evite le repli sur la derivee ecran du shader lit (source de
// scintillement/blanchiment sur les pieces fines comme roues/antenne/echappement).
void DrawCylinderExLit(
    Vector3 startPos, Vector3 endPos,
    float startRadius, float endRadius, int sides, Color color);

// Meme geometrie que raylib::DrawSphereEx, mais emet une normale radiale
// (position locale normalisee) par sommet.
void DrawSphereLit(
    Vector3 centerPos, float radius, int rings, int slices, Color color);

} // namespace racer

#endif /* !CAR_PRIMITIVES_HPP_ */
