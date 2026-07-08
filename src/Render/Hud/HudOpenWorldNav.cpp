/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Open world navigation HUD
*/

#include <cstdio>

#include "Render/Hud/HudOpenWorldNav.hpp"

#include <cmath>
#include <vector>

#include "Render/Hud/HudGfx.hpp"
#include "Render/Hud/HudMinimap.hpp"
#include "World/Aurelia/AureliaBounds.hpp"
#include "World/Aurelia/AureliaData.hpp"
#include "World/Aurelia/AureliaTypes.hpp"

namespace racer {

namespace {

HudMapProjection worldMapProjection(Rectangle area)
{
    using world::WorldBounds;

    HudMapProjection proj{};
    proj.worldCenter = Vector2{WorldBounds::centerX, WorldBounds::centerZ};
    proj.screenCenter = Vector2{area.x + area.width * 0.5f,
        area.y + area.height * 0.5f};
    float pad = 10.0f;
    float scaleX = (area.width - pad * 2.0f) / WorldBounds::width();
    float scaleY = (area.height - pad * 2.0f) / WorldBounds::height();
    proj.scale = std::min(scaleX, scaleY);
    proj.rotated = false;
    return proj;
}

void drawWorldBorder(const HudMapProjection &proj, Rectangle area, float thick,
    Color color)
{
    using world::WorldBounds;

    Vector2 tl = proj.apply(Vector2{WorldBounds::minX, WorldBounds::minZ});
    Vector2 tr = proj.apply(Vector2{WorldBounds::maxX, WorldBounds::minZ});
    Vector2 bl = proj.apply(Vector2{WorldBounds::minX, WorldBounds::maxZ});
    Vector2 br = proj.apply(Vector2{WorldBounds::maxX, WorldBounds::maxZ});

    HudGfx::drawLineEx(tl, tr, thick, color);
    HudGfx::drawLineEx(tr, br, thick, color);
    HudGfx::drawLineEx(br, bl, thick, color);
    HudGfx::drawLineEx(bl, tl, thick, color);

    Rectangle clip = area;
    (void)clip;
}

void drawRoads(const world::RoadGraph &roads, const HudMapProjection &proj,
    float thick, Color color)
{
    for (size_t ei = 0; ei < roads.edges().size(); ++ei) {
        std::vector<Vector2> pts;
        pts.reserve(17);
        for (int s = 0; s <= 16; ++s) {
            float t = static_cast<float>(s) / 16.0f;
            pts.push_back(roads.pointOnEdge(static_cast<int>(ei), t));
        }
        HudMinimap::drawPathPolyline(pts, proj, thick, color);
    }
}

const char *raceMarkerLetter(const world::PoiInstance &poi)
{
    switch (poi.region) {
    case world::RegionId::MARINA:
        return "M";
    case world::RegionId::FOREST:
        return "F";
    case world::RegionId::PORT:
        return "P";
    case world::RegionId::VOLCANO:
        return "V";
    default:
        return "?";
    }
}

void drawRaceMarkers(const world::AureliaWorld &world,
    const HudMapProjection &proj, bool labels)
{
    for (const world::PoiInstance &poi : world.pois()) {
        if (poi.type != world::PoiType::RACE_ENTRY) {
            continue;
        }
        if (poi.trackIndex == 4
            && !world.progression().cendresCircuitUnlocked()) {
            continue;
        }
        Vector2 sp = proj.apply(Vector2{poi.worldX, poi.worldZ});
        HudGfx::drawCircleV(sp, labels ? 6.0f : 5.0f, poi.color);
        HudGfx::drawCircleLinesV(sp, labels ? 7.0f : 6.0f, RAYWHITE);
        if (labels) {
            HudGfx::drawTextCentered(raceMarkerLetter(poi),
                static_cast<int>(sp.x), static_cast<int>(sp.y) - 7, 12, BLACK);
        }
    }
}

void drawGarageMarkers(const world::AureliaWorld &world,
    const HudMapProjection &proj)
{
    for (const world::PoiInstance &poi : world.pois()) {
        if (poi.type != world::PoiType::GARAGE) {
            continue;
        }
        Vector2 sp = proj.apply(Vector2{poi.worldX, poi.worldZ});
        HudGfx::drawCircleV(sp, 3.5f, HudGfx::fade(SKYBLUE, 0.9f));
    }
}

void drawRegionLabels(const HudMapProjection &proj)
{
    struct RegionLabel {
        const char *text;
        float x;
        float z;
        Color color;
    };
    static const RegionLabel regions[] = {
        {"Marina", 16.0f, 4.0f, Color{255, 210, 80, 255}},
        {"Foret", -72.0f, -128.0f, Color{120, 220, 140, 255}},
        {"Port", 168.0f, -32.0f, Color{255, 120, 90, 255}},
        {"Volcan", 72.0f, 168.0f, Color{255, 160, 60, 255}},
    };

    for (const RegionLabel &r : regions) {
        Vector2 sp = proj.apply(Vector2{r.x, r.z});
        HudGfx::drawTextCentered(r.text, static_cast<int>(sp.x),
            static_cast<int>(sp.y) - 6, 11, HudGfx::fade(r.color, 0.85f));
    }
}

void drawPlayerArrow(const HudMapProjection &proj, Vector3 pos, float heading,
    float size, Color color)
{
    Vector2 center = proj.apply(Vector2{pos.x, pos.z});
    float tipX = center.x + std::sin(heading) * size;
    float tipY = center.y - std::cos(heading) * size;
    float leftX = center.x + std::sin(heading + 2.4f) * size * 0.55f;
    float leftY = center.y - std::cos(heading + 2.4f) * size * 0.55f;
    float rightX = center.x + std::sin(heading - 2.4f) * size * 0.55f;
    float rightY = center.y - std::cos(heading - 2.4f) * size * 0.55f;

    HudGfx::drawLineEx(center, Vector2{tipX, tipY}, 2.5f, color);
    HudGfx::drawLineEx(Vector2{tipX, tipY}, Vector2{leftX, leftY}, 2.0f, color);
    HudGfx::drawLineEx(Vector2{tipX, tipY}, Vector2{rightX, rightY}, 2.0f,
        color);
}

void drawNavLine(const HudMapProjection &proj, Vector3 from,
    const world::PoiInstance &target, Color color)
{
    Vector2 a = proj.apply(Vector2{from.x, from.z});
    Vector2 b = proj.apply(Vector2{target.worldX, target.worldZ});
    HudGfx::drawLineEx(a, b, 1.5f, HudGfx::fade(color, 0.55f));
}

const world::PoiInstance *findNearestRace(const world::AureliaWorld &world,
    float &outDist)
{
    const world::PoiInstance *nearest = nullptr;
    float nearestDist = 1.0e9f;
    Vector3 pos = world.playerCar().position();

    for (const world::PoiInstance &poi : world.pois()) {
        if (poi.type != world::PoiType::RACE_ENTRY) {
            continue;
        }
        if (poi.trackIndex == 4
            && !world.progression().cendresCircuitUnlocked()) {
            continue;
        }
        float dx = poi.worldX - pos.x;
        float dz = poi.worldZ - pos.z;
        float d = std::sqrt(dx * dx + dz * dz);
        if (d < nearestDist) {
            nearestDist = d;
            nearest = &poi;
        }
    }
    outDist = nearestDist;
    return nearest;
}

// Race entries win near-ties over missions/collectibles so the compass keeps
// steering players toward circuits first, matching existing HUD behaviour.
const world::PoiInstance *findNearestPoi(const world::AureliaWorld &world,
    float &outDist)
{
    constexpr float kRacePreferenceMargin = 25.0f;

    const world::PoiInstance *nearest = nullptr;
    const world::PoiInstance *nearestRace = nullptr;
    float nearestDist = 1.0e9f;
    float nearestRaceDist = 1.0e9f;
    Vector3 pos = world.playerCar().position();

    for (const world::PoiInstance &poi : world.pois()) {
        bool relevant = poi.type == world::PoiType::RACE_ENTRY
            || poi.type == world::PoiType::MISSION_GIVER
            || poi.type == world::PoiType::COLLECTIBLE;
        if (!relevant) {
            continue;
        }
        if (poi.type == world::PoiType::RACE_ENTRY && poi.trackIndex == 4
            && !world.progression().cendresCircuitUnlocked()) {
            continue;
        }
        float dx = poi.worldX - pos.x;
        float dz = poi.worldZ - pos.z;
        float d = std::sqrt(dx * dx + dz * dz);
        if (d < nearestDist) {
            nearestDist = d;
            nearest = &poi;
        }
        if (poi.type == world::PoiType::RACE_ENTRY && d < nearestRaceDist) {
            nearestRaceDist = d;
            nearestRace = &poi;
        }
    }

    if (nearestRace != nullptr
        && nearestRaceDist <= nearestDist + kRacePreferenceMargin) {
        outDist = nearestRaceDist;
        return nearestRace;
    }
    outDist = nearestDist;
    return nearest;
}

const char *poiKindLabel(world::PoiType type)
{
    switch (type) {
    case world::PoiType::MISSION_GIVER:
        return "Mission";
    case world::PoiType::COLLECTIBLE:
        return "Souvenir";
    default:
        return "Circuit";
    }
}

} // namespace

void HudOpenWorldNav::drawPersistentMinimap(const world::AureliaWorld &world,
    int screenWidth, int screenHeight)
{
    (void)screenWidth;
    constexpr float mapSize = 156.0f;
    Rectangle area{
        14.0f, static_cast<float>(screenHeight) - mapSize - 14.0f, mapSize,
        mapSize};

    HudGfx::drawPanel(area, 0.72f);
    HudGfx::drawRectangleRoundedLinesEx(area, 0.15f, 8, 2.0f, ORANGE);
    HudGfx::drawText("Carte", static_cast<int>(area.x + 8.0f),
        static_cast<int>(area.y + 6.0f), 12, HudGfx::fade(WHITE, 0.75f));

    HudMapProjection proj = worldMapProjection(area);
    drawWorldBorder(proj, area, 2.0f, HudGfx::fade(ORANGE, 0.9f));
    drawRoads(world::AureliaData::roadGraph(), proj, 2.0f,
        HudGfx::fade(Color{170, 170, 180, 255}, 0.8f));
    drawGarageMarkers(world, proj);
    drawRaceMarkers(world, proj, false);

    float nearestDist = 0.0f;
    const world::PoiInstance *nearest = findNearestRace(world, nearestDist);
    if (nearest != nullptr && world.activeRaceIndex() < 0) {
        drawNavLine(proj, world.playerCar().position(), *nearest, YELLOW);
    }

    drawPlayerArrow(proj, world.playerCar().position(),
        world.playerCar().heading(), 8.0f, RED);
}

void HudOpenWorldNav::drawCompass(const world::AureliaWorld &world,
    int screenWidth, int screenHeight)
{
    if (world.activeRaceIndex() >= 0) {
        return;
    }

    float nearestDist = 0.0f;
    const world::PoiInstance *nearest = findNearestPoi(world, nearestDist);
    if (nearest == nullptr) {
        return;
    }

    Vector3 pos = world.playerCar().position();
    float dx = nearest->worldX - pos.x;
    float dz = nearest->worldZ - pos.z;
    float bearing = std::atan2(dx, dz);
    float rel = bearing - world.playerCar().heading();
    while (rel > 3.14159265f) {
        rel -= 6.2831853f;
    }
    while (rel < -3.14159265f) {
        rel += 6.2831853f;
    }

    Vector2 center{
        static_cast<float>(screenWidth) * 0.5f,
        static_cast<float>(screenHeight) - 42.0f};
    float arrowLen = 22.0f;
    float tipX = center.x + std::sin(rel) * arrowLen;
    float tipY = center.y - std::cos(rel) * arrowLen;
    float wing = 10.0f;

    Color c = nearest->color;
    HudGfx::drawLineEx(center, Vector2{tipX, tipY}, 4.0f, c);
    HudGfx::drawLineEx(Vector2{tipX, tipY},
        Vector2{tipX - std::sin(rel + 0.55f) * wing,
            tipY + std::cos(rel + 0.55f) * wing},
        3.0f, c);
    HudGfx::drawLineEx(Vector2{tipX, tipY},
        Vector2{tipX - std::sin(rel - 0.55f) * wing,
            tipY + std::cos(rel - 0.55f) * wing},
        3.0f, c);
    HudGfx::drawCircleV(center, 4.0f, HudGfx::fade(WHITE, 0.8f));

    char buf[96];
    const char *kind = poiKindLabel(nearest->type);
    const char *name = (nearest->label != nullptr && nearest->label[0] != '\0')
        ? nearest->label
        : kind;
    if (nearest->type == world::PoiType::RACE_ENTRY) {
        std::snprintf(buf, sizeof(buf), "%s  %.0f m", name, nearestDist);
    } else {
        std::snprintf(buf, sizeof(buf), "%s : %s  %.0f m", kind, name,
            nearestDist);
    }
    HudGfx::drawTextCentered(buf, screenWidth / 2,
        static_cast<int>(center.y - 34.0f), 15, HudGfx::fade(WHITE, 0.88f));
}

void HudOpenWorldNav::drawEdgeWarning(const world::AureliaWorld &world,
    int screenWidth, int screenHeight)
{
    Vector3 pos = world.playerCar().position();
    float edge = world::distanceToWorldEdge(pos.x, pos.z);
    if (edge > world::WorldBounds::edgeBand) {
        return;
    }

    HudGfx::drawTextCentered("Limite de la peninsule — revenez vers le centre",
        screenWidth / 2, 108, 16, HudGfx::fade(ORANGE, 0.9f));
}

void HudOpenWorldNav::drawExpandedMap(const world::AureliaWorld &world,
    int screenWidth, int screenHeight)
{
    float mapW = static_cast<float>(screenWidth) * 0.42f;
    float mapH = static_cast<float>(screenHeight) * 0.52f;
    Rectangle area{
        static_cast<float>(screenWidth) * 0.5f - mapW * 0.5f,
        static_cast<float>(screenHeight) * 0.5f - mapH * 0.5f, mapW, mapH};

    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.45f));
    HudGfx::drawPanel(area, 0.88f);
    HudGfx::drawRectangleRoundedLinesEx(area, 0.12f, 10, 3.0f, ORANGE);
    HudGfx::drawTextCentered("Carte Aurelia (M pour fermer)",
        screenWidth / 2, static_cast<int>(area.y - 22.0f), 18, ORANGE);

    HudMapProjection proj = worldMapProjection(area);
    drawWorldBorder(proj, area, 3.0f, ORANGE);
    drawRegionLabels(proj);
    drawRoads(world::AureliaData::roadGraph(), proj, 3.5f,
        Color{190, 190, 200, 255});
    drawGarageMarkers(world, proj);
    drawRaceMarkers(world, proj, true);

    for (const world::PoiInstance &poi : world.pois()) {
        if (poi.type == world::PoiType::COLLECTIBLE) {
            Vector2 sp = proj.apply(Vector2{poi.worldX, poi.worldZ});
            HudGfx::drawCircleV(sp, 2.5f, Fade(GOLD, 0.65f));
        }
    }

    float nearestDist = 0.0f;
    const world::PoiInstance *nearest = findNearestRace(world, nearestDist);
    if (nearest != nullptr) {
        drawNavLine(proj, world.playerCar().position(), *nearest, YELLOW);
    }

    drawPlayerArrow(proj, world.playerCar().position(),
        world.playerCar().heading(), 10.0f, RED);

    HudGfx::drawText("M = circuit   F = foret   P = port   V = volcan",
        static_cast<int>(area.x + 10.0f),
        static_cast<int>(area.y + area.height + 8.0f), 12,
        HudGfx::fade(WHITE, 0.7f));
}

} // namespace racer
