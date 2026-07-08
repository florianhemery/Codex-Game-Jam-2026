/*
** EPITECH PROJECT, 2026
** racer
** File description:
** HUD minimap rendering
*/

#include "Render/Hud/HudMinimap.hpp"

#include <algorithm>
#include <cmath>

#include "Render/Hud/HudGfx.hpp"

namespace racer {

Vector2 HudMapProjection::apply(Vector2 world) const
{
    float dx = world.x - worldCenter.x;
    float dz = world.y - worldCenter.y;

    if (rotated) {
        return Vector2{
            screenCenter.x + dz * scale,
            screenCenter.y + dx * scale
        };
    }
    return Vector2{
        screenCenter.x + dx * scale,
        screenCenter.y - dz * scale
    };
}

HudMapProjection HudMinimap::fitTrackInRect(const std::vector<Vector2> &points,
    Rectangle area, bool allowRotate)
{
    Vector2 minPoint{1e9f, 1e9f};
    Vector2 maxPoint{-1e9f, -1e9f};

    for (const Vector2 &point : points) {
        minPoint.x = std::min(minPoint.x, point.x);
        minPoint.y = std::min(minPoint.y, point.y);
        maxPoint.x = std::max(maxPoint.x, point.x);
        maxPoint.y = std::max(maxPoint.y, point.y);
    }
    float bw = std::max(maxPoint.x - minPoint.x, 0.001f);
    float bh = std::max(maxPoint.y - minPoint.y, 0.001f);

    HudMapProjection proj;
    proj.worldCenter = Vector2{
        (minPoint.x + maxPoint.x) * 0.5f,
        (minPoint.y + maxPoint.y) * 0.5f
    };
    proj.screenCenter = Vector2{
        area.x + area.width * 0.5f,
        area.y + area.height * 0.5f
    };
    float scaleStraight = std::min(area.width / bw, area.height / bh);
    float scaleRotated = std::min(area.width / bh, area.height / bw);

    proj.rotated = allowRotate && scaleRotated > scaleStraight;
    proj.scale = proj.rotated ? scaleRotated : scaleStraight;
    return proj;
}

void HudMinimap::drawTrackPolyline(const std::vector<Vector2> &points,
    const HudMapProjection &proj, float thickness, Color color)
{
    size_t count = points.size();

    for (size_t i = 0; i < count; ++i) {
        Vector2 a = proj.apply(points[i]);
        Vector2 b = proj.apply(points[(i + 1) % count]);

        HudGfx::drawLineEx(a, b, thickness, color);
        HudGfx::drawCircleV(a, thickness * 0.5f, color);
    }
}

void HudMinimap::drawFinishLineTick(const std::vector<Vector2> &points,
    const HudMapProjection &proj, float halfLength, Color color)
{
    if (points.size() < 2) {
        return;
    }
    Vector2 a = proj.apply(points[0]);
    Vector2 b = proj.apply(points[1]);
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float len = std::sqrt(dx * dx + dy * dy);

    if (len < 0.0001f) {
        return;
    }
    Vector2 perp{-dy / len, dx / len};

    HudGfx::drawLineEx(
        Vector2{a.x + perp.x * halfLength, a.y + perp.y * halfLength},
        Vector2{a.x - perp.x * halfLength, a.y - perp.y * halfLength},
        3.0f, color);
}

void HudMinimap::drawOpponents(const RaceState &race, const HudExtras &extras,
    const HudMapProjection &proj, const Rectangle &panel)
{
    const std::vector<RacerEntry> &racers = race.racers();

    for (size_t i = 0; i < racers.size(); ++i) {
        if (racers[i].isPlayer) {
            continue;
        }
        Vector2 pos = proj.apply(Vector2{
            racers[i].car.position().x,
            racers[i].car.position().z
        });
        pos.x = std::clamp(pos.x, panel.x + 10.0f,
            panel.x + panel.width - 10.0f);
        pos.y = std::clamp(pos.y, panel.y + 10.0f,
            panel.y + panel.height - 10.0f);
        HudGfx::drawCircleV(pos, 4.5f, HudGfx::racerColorFor(extras, i, false));
        HudGfx::drawCircleLinesV(pos, 4.5f, HudGfx::fade(BLACK, 0.55f));
    }
}

void HudMinimap::drawPlayer(const RaceState &race, const HudExtras &extras,
    const HudMapProjection &proj, const Rectangle &panel)
{
    const std::vector<RacerEntry> &racers = race.racers();
    size_t playerIdx = static_cast<size_t>(race.playerIndex());
    Vector2 pos = proj.apply(Vector2{
        racers[playerIdx].car.position().x,
        racers[playerIdx].car.position().z
    });

    pos.x = std::clamp(pos.x, panel.x + 10.0f, panel.x + panel.width - 10.0f);
    pos.y = std::clamp(pos.y, panel.y + 10.0f, panel.y + panel.height - 10.0f);
    HudGfx::drawCircleV(pos, 6.0f, HudGfx::racerColorFor(extras, playerIdx, true));
    HudGfx::drawRing(pos, 7.5f, 9.5f, 0.0f, 360.0f, 24, WHITE);
}

void HudMinimap::draw(const RaceState &race, const HudExtras &extras,
    int screenWidth, int screenHeight)
{
    float screenMin = std::min(
        static_cast<float>(screenWidth), static_cast<float>(screenHeight));
    const float size = std::clamp(screenMin * 0.32f, 260.0f, 360.0f);
    Rectangle panel{
        static_cast<float>(screenWidth) - size - 16.0f,
        static_cast<float>(screenHeight) - size - 16.0f,
        size, size
    };

    HudGfx::drawPanel(panel, 0.45f);

    const std::vector<Vector2> &waypoints = race.getTrack().waypoints();

    if (waypoints.size() < 2) {
        return;
    }
    Rectangle area{
        panel.x + 12.0f, panel.y + 12.0f,
        panel.width - 24.0f, panel.height - 24.0f
    };
    HudMapProjection proj = HudMinimap::fitTrackInRect(waypoints, area, true);

    float road = std::clamp(race.getTrack().width() * proj.scale, 5.0f, 16.0f);

    HudMinimap::drawTrackPolyline(waypoints, proj, road, HudGfx::fade(WHITE, 0.32f));
    HudMinimap::drawTrackPolyline(waypoints, proj, 2.0f, HudGfx::fade(WHITE, 0.55f));
    HudMinimap::drawFinishLineTick(waypoints, proj, road * 0.8f + 2.0f, RAYWHITE);
    HudMinimap::drawOpponents(race, extras, proj, panel);
    HudMinimap::drawPlayer(race, extras, proj, panel);
}

} // namespace racer
