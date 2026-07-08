/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Open world navigation HUD — minimap, compass, expanded map
*/

#ifndef HUD_OPEN_WORLD_NAV_HPP_
#define HUD_OPEN_WORLD_NAV_HPP_

#include "World/Aurelia/AureliaWorld.hpp"

namespace racer {

struct HudOpenWorldNav {
    static void drawPersistentMinimap(const world::AureliaWorld &world,
        int screenWidth, int screenHeight);
    static void drawCompass(const world::AureliaWorld &world, int screenWidth,
        int screenHeight);
    static void drawEdgeWarning(const world::AureliaWorld &world,
        int screenWidth, int screenHeight);
    static void drawExpandedMap(const world::AureliaWorld &world,
        int screenWidth, int screenHeight);
};

} // namespace racer

#endif /* !HUD_OPEN_WORLD_NAV_HPP_ */
