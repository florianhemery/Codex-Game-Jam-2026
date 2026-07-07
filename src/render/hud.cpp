#include "render/hud.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

#include "raylib.h"

namespace racer {

namespace {

// ---------------------------------------------------------------------------
// Utilitaires texte / couleur
// ---------------------------------------------------------------------------

// Ordinal feminin, accorde avec "place" (1re, 2e, ...).
void FormatOrdinal(int position, char* buf, size_t bufSize) {
    if (position == 1) {
        std::snprintf(buf, bufSize, "1re");
    } else {
        std::snprintf(buf, bufSize, "%de", position);
    }
}

// Format m:ss.cc (ex : 1:02.34).
void FormatTime(float seconds, char* buf, size_t bufSize) {
    int m = static_cast<int>(seconds) / 60;
    float s = seconds - static_cast<float>(m * 60);
    std::snprintf(buf, bufSize, "%d:%05.2f", m, s);
}

// Comme FormatTime, mais 0 signifie "pas encore de donnee" -> "--".
void FormatLapTime(float seconds, char* buf, size_t bufSize) {
    if (seconds <= 0.0f) {
        std::snprintf(buf, bufSize, "--");
    } else {
        FormatTime(seconds, buf, bufSize);
    }
}

Color LerpColor(Color a, Color b, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    auto mix = [t](unsigned char x, unsigned char y) {
        return static_cast<unsigned char>(static_cast<float>(x) + (static_cast<float>(y) - static_cast<float>(x)) * t);
    };
    return Color{mix(a.r, b.r), mix(a.g, b.g), mix(a.b, b.b), mix(a.a, b.a)};
}

// Couleur d'un coureur : fournie par extras si presente, sinon palette de
// secours alignee sur les couleurs des voitures de main.cpp (joueur rouge).
Color RacerColorFor(const HudExtras& extras, size_t index, bool isPlayer) {
    if (index < extras.racerColors.size()) return extras.racerColors[index];
    if (isPlayer) return RED;
    constexpr Color kFallback[] = {BLUE, DARKGREEN, ORANGE, PURPLE, SKYBLUE, MAROON};
    return kFallback[index % (sizeof(kFallback) / sizeof(kFallback[0]))];
}

void DrawPanel(Rectangle rect, float alpha) {
    DrawRectangleRounded(rect, 0.12f, 8, Fade(BLACK, alpha));
    DrawRectangleRoundedLinesEx(rect, 0.12f, 8, 1.0f, Fade(WHITE, 0.10f));
}

void DrawTextCentered(const char* text, int centerX, int y, int fontSize, Color color) {
    DrawText(text, centerX - MeasureText(text, fontSize) / 2, y, fontSize, color);
}

void DrawTextRightAligned(const char* text, int rightX, int y, int fontSize, Color color) {
    DrawText(text, rightX - MeasureText(text, fontSize), y, fontSize, color);
}

void DrawTextShadowCentered(const char* text, int centerX, int y, int fontSize, Color color, int offset) {
    int w = MeasureText(text, fontSize);
    DrawText(text, centerX - w / 2 + offset, y + offset, fontSize, Fade(BLACK, 0.55f));
    DrawText(text, centerX - w / 2, y, fontSize, color);
}

// Texte multi-lignes coupe aux mots (buffers sur pile, aucune allocation).
void DrawTextWrapped(const char* text, int x, int y, int maxWidth, int fontSize, int lineHeight, Color color) {
    char line[192] = {0};
    int lineLen = 0;
    int lines = 0;
    const char* p = text;

    while (true) {
        while (*p == ' ') ++p;
        if (*p == '\0') break;

        char word[96];
        int wordLen = 0;
        while (*p != '\0' && *p != ' ' && wordLen < 95) word[wordLen++] = *p++;
        word[wordLen] = '\0';

        char candidate[192];
        if (lineLen == 0) {
            std::snprintf(candidate, sizeof(candidate), "%s", word);
        } else {
            std::snprintf(candidate, sizeof(candidate), "%s %s", line, word);
        }

        if (lineLen > 0 && MeasureText(candidate, fontSize) > maxWidth) {
            DrawText(line, x, y + lines * lineHeight, fontSize, color);
            ++lines;
            lineLen = std::snprintf(line, sizeof(line), "%s", word);
        } else {
            lineLen = std::snprintf(line, sizeof(line), "%s", candidate);
        }
    }
    if (lineLen > 0) {
        DrawText(line, x, y + lines * lineHeight, fontSize, color);
    }
}

// ---------------------------------------------------------------------------
// Projection piste -> ecran (minimap et apercus du menu)
// ---------------------------------------------------------------------------

struct MapProjection {
    Vector2 screenCenter{};
    Vector2 worldCenter{};
    float scale = 1.0f;
    bool rotated = false; // true : Z monde -> X ecran (orientation paysage)

    Vector2 Apply(Vector2 world) const {
        float dx = world.x - worldCenter.x;
        float dz = world.y - worldCenter.y;
        if (rotated) return Vector2{screenCenter.x + dz * scale, screenCenter.y + dx * scale};
        return Vector2{screenCenter.x + dx * scale, screenCenter.y - dz * scale};
    }
};

// Normalise/centre la piste dans une zone, aspect preserve. allowRotate
// autorise a coucher la piste (paysage) si cela remplit mieux la zone.
MapProjection FitTrackInRect(const std::vector<Vector2>& points, Rectangle area, bool allowRotate) {
    Vector2 mn{1e9f, 1e9f};
    Vector2 mx{-1e9f, -1e9f};
    for (const Vector2& p : points) {
        mn.x = std::min(mn.x, p.x);
        mn.y = std::min(mn.y, p.y);
        mx.x = std::max(mx.x, p.x);
        mx.y = std::max(mx.y, p.y);
    }
    float bw = std::max(mx.x - mn.x, 0.001f);
    float bh = std::max(mx.y - mn.y, 0.001f);

    MapProjection proj;
    proj.worldCenter = Vector2{(mn.x + mx.x) * 0.5f, (mn.y + mx.y) * 0.5f};
    proj.screenCenter = Vector2{area.x + area.width * 0.5f, area.y + area.height * 0.5f};
    float scaleStraight = std::min(area.width / bw, area.height / bh);
    float scaleRotated = std::min(area.width / bh, area.height / bw);
    proj.rotated = allowRotate && scaleRotated > scaleStraight;
    proj.scale = proj.rotated ? scaleRotated : scaleStraight;
    return proj;
}

void DrawTrackPolyline(const std::vector<Vector2>& points, const MapProjection& proj, float thickness, Color color) {
    size_t n = points.size();
    for (size_t i = 0; i < n; ++i) {
        Vector2 a = proj.Apply(points[i]);
        Vector2 b = proj.Apply(points[(i + 1) % n]);
        DrawLineEx(a, b, thickness, color);
        DrawCircleV(a, thickness * 0.5f, color); // arrondit les jonctions
    }
}

// Petit trait perpendiculaire a la piste au niveau de la ligne d'arrivee (waypoint 0).
void DrawFinishLineTick(const std::vector<Vector2>& points, const MapProjection& proj, float halfLength, Color color) {
    if (points.size() < 2) return;
    Vector2 a = proj.Apply(points[0]);
    Vector2 b = proj.Apply(points[1]);
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 0.0001f) return;
    Vector2 perp{-dy / len, dx / len};
    DrawLineEx(Vector2{a.x + perp.x * halfLength, a.y + perp.y * halfLength},
               Vector2{a.x - perp.x * halfLength, a.y - perp.y * halfLength}, 3.0f, color);
}

// ---------------------------------------------------------------------------
// Panneaux du HUD course
// ---------------------------------------------------------------------------

void DrawMinimap(const RaceState& race, const HudExtras& extras, int screenWidth, int screenHeight) {
    const float size = 220.0f;
    Rectangle panel{static_cast<float>(screenWidth) - size - 16.0f,
                    static_cast<float>(screenHeight) - size - 16.0f, size, size};
    DrawPanel(panel, 0.45f);

    const std::vector<Vector2>& wp = race.GetTrack().Waypoints();
    if (wp.size() < 2) return;

    Rectangle area{panel.x + 20.0f, panel.y + 20.0f, panel.width - 40.0f, panel.height - 40.0f};
    MapProjection proj = FitTrackInRect(wp, area, false); // portrait, nord en haut

    float road = std::clamp(race.GetTrack().Width() * proj.scale, 3.0f, 10.0f);
    DrawTrackPolyline(wp, proj, road, Fade(WHITE, 0.15f));
    DrawTrackPolyline(wp, proj, 1.5f, Fade(WHITE, 0.28f));
    DrawFinishLineTick(wp, proj, road * 0.8f + 2.0f, RAYWHITE);

    const std::vector<RacerEntry>& racers = race.Racers();
    for (size_t i = 0; i < racers.size(); ++i) {
        if (racers[i].isPlayer) continue;
        Vector2 p = proj.Apply(Vector2{racers[i].car.position.x, racers[i].car.position.z});
        p.x = std::clamp(p.x, panel.x + 10.0f, panel.x + panel.width - 10.0f);
        p.y = std::clamp(p.y, panel.y + 10.0f, panel.y + panel.height - 10.0f);
        DrawCircleV(p, 4.5f, RacerColorFor(extras, i, false));
        DrawCircleLinesV(p, 4.5f, Fade(BLACK, 0.55f));
    }

    // Joueur dessine en dernier : point plus gros + anneau blanc.
    size_t playerIdx = static_cast<size_t>(race.PlayerIndex());
    Vector2 p = proj.Apply(Vector2{racers[playerIdx].car.position.x, racers[playerIdx].car.position.z});
    p.x = std::clamp(p.x, panel.x + 10.0f, panel.x + panel.width - 10.0f);
    p.y = std::clamp(p.y, panel.y + 10.0f, panel.y + panel.height - 10.0f);
    DrawCircleV(p, 6.0f, RacerColorFor(extras, playerIdx, true));
    DrawRing(p, 7.5f, 9.5f, 0.0f, 360.0f, 24, WHITE);
}

void DrawSpeedGauge(const Car& car, int screenHeight) {
    const float panelW = 272.0f;
    const float panelH = 232.0f;
    Rectangle panel{16.0f, static_cast<float>(screenHeight) - panelH - 16.0f, panelW, panelH};
    DrawPanel(panel, 0.50f);

    const Vector2 center{panel.x + panelW * 0.5f, panel.y + 108.0f};
    const float rOut = 82.0f;
    const float rIn = 70.0f;
    const float angleMin = 150.0f; // arc de 240 degres ouvert vers le bas
    const float angleSpan = 240.0f;
    const float kmhMax = 230.0f;

    float kmh = std::fabs(car.speed) * 6.0f; // meme convention d'affichage que l'ancien HUD
    float ratio = std::clamp(kmh / kmhMax, 0.0f, 1.0f);

    DrawRing(center, rIn, rOut, angleMin, angleMin + angleSpan, 64, Fade(WHITE, 0.08f));
    float redlineStart = angleMin + angleSpan * (200.0f / kmhMax);
    DrawRing(center, rIn, rOut, redlineStart, angleMin + angleSpan, 16, Fade(RED, 0.20f));
    if (ratio > 0.004f) {
        Color fill = LerpColor(Color{255, 168, 40, 255}, Color{255, 66, 40, 255}, ratio);
        DrawRing(center, rIn + 1.0f, rOut - 1.0f, angleMin, angleMin + angleSpan * ratio, 64, fill);
    }

    // Graduations tous les 10 km/h, majeures (avec chiffre) tous les 50.
    for (int v = 0; v <= static_cast<int>(kmhMax); v += 10) {
        bool major = (v % 50) == 0;
        float a = (angleMin + angleSpan * static_cast<float>(v) / kmhMax) * DEG2RAD;
        Vector2 dir{std::cos(a), std::sin(a)};
        float r1 = rOut + 3.0f;
        float r2 = rOut + (major ? 10.0f : 6.0f);
        DrawLineEx(Vector2{center.x + dir.x * r1, center.y + dir.y * r1},
                   Vector2{center.x + dir.x * r2, center.y + dir.y * r2},
                   major ? 2.0f : 1.0f, Fade(WHITE, major ? 0.70f : 0.32f));
        if (major) {
            char label[8];
            std::snprintf(label, sizeof(label), "%d", v);
            float rl = rOut + 20.0f;
            DrawTextCentered(label, static_cast<int>(center.x + dir.x * rl),
                             static_cast<int>(center.y + dir.y * rl) - 5, 10, Fade(WHITE, 0.55f));
        }
    }

    // Aiguille (n'atteint pas le centre pour laisser la place aux chiffres).
    {
        float a = (angleMin + angleSpan * ratio) * DEG2RAD;
        Vector2 dir{std::cos(a), std::sin(a)};
        DrawLineEx(Vector2{center.x + dir.x * 30.0f, center.y + dir.y * 30.0f},
                   Vector2{center.x + dir.x * (rIn - 3.0f), center.y + dir.y * (rIn - 3.0f)},
                   3.0f, Color{255, 92, 70, 255});
        DrawCircleV(Vector2{center.x + dir.x * (rIn - 3.0f), center.y + dir.y * (rIn - 3.0f)}, 2.5f, RAYWHITE);
    }

    char speedBuf[16];
    std::snprintf(speedBuf, sizeof(speedBuf), "%.0f", kmh);
    DrawTextCentered(speedBuf, static_cast<int>(center.x), static_cast<int>(center.y) - 16, 40, RAYWHITE);
    DrawTextCentered("km/h", static_cast<int>(center.x), static_cast<int>(center.y) + 28, 14, Fade(WHITE, 0.60f));

    // Barre de nitro sous l'arc.
    float nitroRatio = std::clamp(car.nitroRemaining / car.tuning.nitroCapacity, 0.0f, 1.0f);
    float barY = panel.y + panelH - 30.0f;
    Rectangle barBg{panel.x + 86.0f, barY, panelW - 86.0f - 18.0f, 13.0f};
    DrawText("NITRO", static_cast<int>(panel.x + 20.0f), static_cast<int>(barY), 13, Fade(ORANGE, 0.95f));
    DrawRectangleRounded(barBg, 0.7f, 6, Fade(WHITE, 0.10f));
    if (nitroRatio > 0.01f) {
        Color col = ORANGE;
        if (nitroRatio >= 0.999f) {
            // Legere pulsation quand la reserve est pleine.
            float pulse = 0.5f + 0.5f * std::sin(static_cast<float>(GetTime()) * 6.0f);
            col = LerpColor(ORANGE, Color{255, 236, 120, 255}, pulse * 0.65f);
        }
        Rectangle fill{barBg.x, barBg.y, barBg.width * nitroRatio, barBg.height};
        DrawRectangleRounded(fill, 0.7f, 6, col);
    }
}

void DrawStandingsPanel(const RaceState& race, const HudExtras& extras) {
    const std::vector<RacerEntry>& racers = race.Racers();
    const RacerEntry& player = racers[static_cast<size_t>(race.PlayerIndex())];
    std::vector<int> order = race.Standings();

    const float rowH = 28.0f;
    Rectangle panel{16.0f, 16.0f, 212.0f, 50.0f + rowH * static_cast<float>(racers.size()) + 10.0f};
    DrawPanel(panel, 0.45f);

    char lapBuf[32];
    std::snprintf(lapBuf, sizeof(lapBuf), "TOUR %d/%d", std::min(player.lap + 1, race.LapsToWin()), race.LapsToWin());
    DrawText(lapBuf, static_cast<int>(panel.x + 14.0f), static_cast<int>(panel.y + 12.0f), 20, RAYWHITE);
    DrawLineEx(Vector2{panel.x + 12.0f, panel.y + 42.0f},
               Vector2{panel.x + panel.width - 12.0f, panel.y + 42.0f}, 1.0f, Fade(WHITE, 0.15f));

    for (size_t i = 0; i < order.size(); ++i) {
        size_t idx = static_cast<size_t>(order[i]);
        const RacerEntry& r = racers[idx];
        float y = panel.y + 50.0f + rowH * static_cast<float>(i);

        if (r.isPlayer) {
            Rectangle hl{panel.x + 8.0f, y - 3.0f, panel.width - 16.0f, rowH - 2.0f};
            DrawRectangleRounded(hl, 0.4f, 6, Fade(YELLOW, 0.16f));
        }

        char pos[8];
        std::snprintf(pos, sizeof(pos), "%d", static_cast<int>(i) + 1);
        DrawTextRightAligned(pos, static_cast<int>(panel.x + 34.0f), static_cast<int>(y), 18,
                             r.isPlayer ? YELLOW : Fade(WHITE, 0.80f));

        Vector2 dot{panel.x + 50.0f, y + 9.0f};
        DrawCircleV(dot, 6.0f, RacerColorFor(extras, idx, r.isPlayer));
        DrawCircleLinesV(dot, 6.0f, Fade(WHITE, 0.35f));

        DrawText(r.name.c_str(), static_cast<int>(panel.x + 66.0f), static_cast<int>(y), 18,
                 r.isPlayer ? YELLOW : RAYWHITE);

        if (r.finished) {
            // Mini drapeau a damier pour les coureurs arrives.
            int fx = static_cast<int>(panel.x + panel.width - 26.0f);
            int fy = static_cast<int>(y + 3.0f);
            DrawRectangle(fx, fy, 5, 5, RAYWHITE);
            DrawRectangle(fx + 5, fy + 5, 5, 5, RAYWHITE);
            DrawRectangle(fx + 5, fy, 5, 5, Fade(WHITE, 0.25f));
            DrawRectangle(fx, fy + 5, 5, 5, Fade(WHITE, 0.25f));
        }
    }
}

void DrawTimersPanel(const RaceState& race, const HudExtras& extras, int screenWidth) {
    // Le dernier tour reste affiche ~3 s apres un passage de ligne.
    bool showLast = extras.lastLapTime > 0.0f && extras.currentLapTime < 3.0f;

    const float panelW = 240.0f;
    float panelH = showLast ? 158.0f : 134.0f;
    Rectangle panel{static_cast<float>(screenWidth) - panelW - 16.0f, 16.0f, panelW, panelH};
    DrawPanel(panel, 0.45f);

    DrawText("TEMPS DE COURSE", static_cast<int>(panel.x + 14.0f), static_cast<int>(panel.y + 10.0f), 12,
             Fade(WHITE, 0.55f));
    char total[32];
    FormatTime(race.ElapsedTime(), total, sizeof(total));
    DrawText(total, static_cast<int>(panel.x + 14.0f), static_cast<int>(panel.y + 26.0f), 30, RAYWHITE);

    DrawLineEx(Vector2{panel.x + 12.0f, panel.y + 64.0f},
               Vector2{panel.x + panel.width - 12.0f, panel.y + 64.0f}, 1.0f, Fade(WHITE, 0.15f));

    char cur[32];
    FormatLapTime(extras.currentLapTime, cur, sizeof(cur));
    DrawText("Tour", static_cast<int>(panel.x + 14.0f), static_cast<int>(panel.y + 72.0f), 16, Fade(WHITE, 0.65f));
    DrawTextRightAligned(cur, static_cast<int>(panel.x + panel.width - 14.0f), static_cast<int>(panel.y + 72.0f), 16,
                         RAYWHITE);

    char best[32];
    FormatLapTime(extras.bestLapTime, best, sizeof(best));
    DrawText("Meilleur", static_cast<int>(panel.x + 14.0f), static_cast<int>(panel.y + 96.0f), 16, Fade(WHITE, 0.65f));
    DrawTextRightAligned(best, static_cast<int>(panel.x + panel.width - 14.0f), static_cast<int>(panel.y + 96.0f), 16,
                         Color{170, 220, 255, 255});

    if (showLast) {
        char last[32];
        FormatTime(extras.lastLapTime, last, sizeof(last));
        float blink = 0.65f + 0.35f * std::sin(static_cast<float>(GetTime()) * 8.0f);
        DrawText("Dernier", static_cast<int>(panel.x + 14.0f), static_cast<int>(panel.y + 120.0f), 16,
                 Fade(YELLOW, blink * 0.8f));
        DrawTextRightAligned(last, static_cast<int>(panel.x + panel.width - 14.0f),
                             static_cast<int>(panel.y + 120.0f), 16, Fade(YELLOW, blink));
    }
}

// ---------------------------------------------------------------------------
// Countdown / depart
// ---------------------------------------------------------------------------

void DrawStartLights(int centerX, int y, int litCount, float alpha) {
    Rectangle band{static_cast<float>(centerX) - 96.0f, static_cast<float>(y), 192.0f, 56.0f};
    DrawRectangleRounded(band, 0.5f, 8, Fade(BLACK, 0.50f * alpha));
    const Color onColors[3] = {Color{235, 64, 52, 255}, Color{235, 64, 52, 255}, Color{86, 225, 104, 255}};
    for (int i = 0; i < 3; ++i) {
        Vector2 p{static_cast<float>(centerX) + static_cast<float>(i - 1) * 56.0f, band.y + 28.0f};
        bool on = i < litCount;
        if (on) DrawCircleV(p, 21.0f, Fade(onColors[i], 0.30f * alpha)); // halo
        DrawCircleV(p, 14.0f, on ? Fade(onColors[i], alpha) : Fade(WHITE, 0.08f * alpha));
        DrawCircleLinesV(p, 14.5f, Fade(WHITE, 0.22f * alpha));
    }
}

void DrawCountdown(const RaceState& race, int screenWidth, int screenHeight) {
    float remaining = race.CountdownRemaining();
    int digit = std::clamp(static_cast<int>(std::ceil(remaining)), 1, 3);
    // Feux progressifs : 3 -> premier rouge, 2 et 1 -> deux rouges. Le vert
    // ne s'allume qu'au depart (voir DrawGoFlash).
    int lit = digit >= 3 ? 1 : 2;
    DrawStartLights(screenWidth / 2, static_cast<int>(static_cast<float>(screenHeight) * 0.15f), lit, 1.0f);

    // Effet "pop" : le chiffre apparait gros puis retombe a l'echelle 1.
    float age = std::clamp(static_cast<float>(digit) - remaining, 0.0f, 1.0f);
    float k = 1.0f - age;
    float scale = 1.0f + 0.5f * k * k * k;
    int fontSize = static_cast<int>(112.0f * scale);
    char buf[4];
    std::snprintf(buf, sizeof(buf), "%d", digit);
    int y = static_cast<int>(static_cast<float>(screenHeight) * 0.40f) - fontSize / 2;
    DrawTextShadowCentered(buf, screenWidth / 2, y, fontSize, RAYWHITE, 5);
}

// "GO !" flashe brievement juste apres le passage en course.
void DrawGoFlash(const RaceState& race, int screenWidth, int screenHeight) {
    const float duration = 1.2f;
    float t = race.ElapsedTime();
    if (t >= duration) return;
    float alpha = 1.0f - t / duration;
    DrawStartLights(screenWidth / 2, static_cast<int>(static_cast<float>(screenHeight) * 0.15f), 3, alpha);
    float flash = 0.72f + 0.28f * std::sin(t * 24.0f);
    const int fontSize = 116;
    int y = static_cast<int>(static_cast<float>(screenHeight) * 0.40f) - fontSize / 2;
    DrawTextShadowCentered("GO !", screenWidth / 2, y, fontSize, Fade(Color{92, 230, 110, 255}, alpha * flash), 5);
}

// ---------------------------------------------------------------------------
// Ecran d'arrivee
// ---------------------------------------------------------------------------

// Ecart estime pour un coureur pas encore arrive : distance restante / vitesse.
bool EstimateGapSeconds(const RaceState& race, const RacerEntry& r, float* outSeconds) {
    const Track& track = race.GetTrack();
    Track::Progress prog = track.ProjectPosition(r.car.position);
    float done = static_cast<float>(r.lap) * track.TotalLength() + track.CumulativeDistance(prog);
    float remaining = static_cast<float>(race.LapsToWin()) * track.TotalLength() - done;
    if (remaining <= 0.0f) {
        *outSeconds = 0.0f;
        return true;
    }
    float speed = std::fabs(r.car.speed);
    if (speed < 2.0f) return false; // quasi arrete : estimation sans interet
    float est = remaining / speed;
    if (est > 120.0f) return false;
    *outSeconds = est;
    return true;
}

void DrawFinishScreen(const RaceState& race, const HudExtras& extras, int screenWidth, int screenHeight) {
    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.65f));

    const std::vector<RacerEntry>& racers = race.Racers();
    std::vector<int> order = race.Standings();

    const float rowH = 30.0f;
    const float panelW = 540.0f;
    const float headerH = 104.0f;
    float panelH = headerH + rowH * static_cast<float>(racers.size()) + 100.0f;
    // Panneau decale vers le haut : laisse la bande sous le tableau libre
    // (main.cpp y ajoute son propre rappel "M : retour au menu").
    Rectangle panel{(static_cast<float>(screenWidth) - panelW) * 0.5f,
                    (static_cast<float>(screenHeight) - panelH) * 0.5f - 30.0f, panelW, panelH};
    DrawRectangleRounded(panel, 0.08f, 8, Fade(BLACK, 0.55f));
    DrawRectangleRoundedLinesEx(panel, 0.08f, 8, 2.0f, Fade(YELLOW, 0.35f));

    char ordinal[16];
    FormatOrdinal(race.PlayerPosition(), ordinal, sizeof(ordinal));
    char title[64];
    std::snprintf(title, sizeof(title), "ARRIVEE -- %s place", ordinal);
    DrawTextShadowCentered(title, static_cast<int>(panel.x + panelW * 0.5f), static_cast<int>(panel.y + 20.0f), 34,
                           YELLOW, 3);

    const RacerEntry& player = racers[static_cast<size_t>(race.PlayerIndex())];
    char timeBuf[32];
    FormatTime(player.finished ? player.finishTime : race.ElapsedTime(), timeBuf, sizeof(timeBuf));
    char timeLine[64];
    std::snprintf(timeLine, sizeof(timeLine), "Temps de course : %s", timeBuf);
    DrawTextCentered(timeLine, static_cast<int>(panel.x + panelW * 0.5f), static_cast<int>(panel.y + 66.0f), 18,
                     Fade(WHITE, 0.85f));

    DrawLineEx(Vector2{panel.x + 24.0f, panel.y + 94.0f}, Vector2{panel.x + panelW - 24.0f, panel.y + 94.0f}, 1.0f,
               Fade(WHITE, 0.20f));

    for (size_t i = 0; i < order.size(); ++i) {
        size_t idx = static_cast<size_t>(order[i]);
        const RacerEntry& r = racers[idx];
        float y = panel.y + headerH + rowH * static_cast<float>(i);

        if (r.isPlayer) {
            Rectangle hl{panel.x + 16.0f, y - 4.0f, panelW - 32.0f, rowH - 2.0f};
            DrawRectangleRounded(hl, 0.4f, 6, Fade(YELLOW, 0.14f));
        }

        char pos[8];
        std::snprintf(pos, sizeof(pos), "%d", static_cast<int>(i) + 1);
        DrawTextRightAligned(pos, static_cast<int>(panel.x + 46.0f), static_cast<int>(y), 20,
                             r.isPlayer ? YELLOW : Fade(WHITE, 0.85f));

        Vector2 dot{panel.x + 66.0f, y + 10.0f};
        DrawCircleV(dot, 6.5f, RacerColorFor(extras, idx, r.isPlayer));
        DrawCircleLinesV(dot, 6.5f, Fade(WHITE, 0.35f));

        DrawText(r.name.c_str(), static_cast<int>(panel.x + 84.0f), static_cast<int>(y), 20,
                 r.isPlayer ? YELLOW : RAYWHITE);

        char right[32];
        if (r.finished) {
            FormatTime(r.finishTime, right, sizeof(right));
        } else {
            float gap = 0.0f;
            if (EstimateGapSeconds(race, r, &gap)) {
                std::snprintf(right, sizeof(right), "+%.2fs", gap);
            } else {
                std::snprintf(right, sizeof(right), "en course");
            }
        }
        DrawTextRightAligned(right, static_cast<int>(panel.x + panelW - 24.0f), static_cast<int>(y), 20,
                             r.finished ? RAYWHITE : Fade(WHITE, 0.60f));
    }

    DrawTextCentered("R rejouer      M menu", static_cast<int>(panel.x + panelW * 0.5f),
                     static_cast<int>(panel.y + panelH - 40.0f), 20, LIGHTGRAY);
}

// ---------------------------------------------------------------------------
// Menu : cartes de circuits avec apercu du trace
// ---------------------------------------------------------------------------

struct TrackPreview {
    std::string key;
    std::vector<Vector2> points;
};

// Apercus construits UNE fois par preset (Track::Make est trop couteux pour
// etre appele chaque frame). La reference retournee est consommee avant tout
// nouvel appel, donc une eventuelle reallocation du cache est sans danger.
const TrackPreview& GetTrackPreview(const TrackDef& def) {
    static std::vector<TrackPreview> cache;
    for (const TrackPreview& entry : cache) {
        if (entry.key == def.name) return entry;
    }
    TrackPreview entry;
    entry.key = def.name;
    entry.points = Track::Make(def).Waypoints();
    cache.push_back(std::move(entry));
    return cache.back();
}

void DrawTrackCard(const TrackDef& def, Rectangle card, bool selected) {
    DrawRectangleRounded(card, 0.10f, 8, Fade(BLACK, selected ? 0.55f : 0.42f));
    if (selected) {
        DrawRectangleRoundedLinesEx(card, 0.10f, 8, 3.0f, YELLOW);
        Rectangle glow{card.x - 4.0f, card.y - 4.0f, card.width + 8.0f, card.height + 8.0f};
        DrawRectangleRoundedLinesEx(glow, 0.10f, 8, 1.0f, Fade(YELLOW, 0.30f));
    } else {
        DrawRectangleRoundedLinesEx(card, 0.10f, 8, 1.0f, Fade(WHITE, 0.12f));
    }

    // Encart d'apercu du trace.
    float insetW = card.width - 90.0f;
    float insetH = insetW * 0.70f;
    Rectangle inset{card.x + (card.width - insetW) * 0.5f, card.y + 16.0f, insetW, insetH};
    DrawRectangleRounded(inset, 0.12f, 6, Fade(BLACK, 0.40f));
    DrawRectangleRoundedLinesEx(inset, 0.12f, 6, 1.0f, Fade(WHITE, 0.10f));

    const TrackPreview& preview = GetTrackPreview(def);
    if (preview.points.size() >= 2) {
        Rectangle area{inset.x + 10.0f, inset.y + 10.0f, inset.width - 20.0f, inset.height - 20.0f};
        MapProjection proj = FitTrackInRect(preview.points, area, true); // couche en paysage
        float road = std::clamp(def.width * proj.scale, 2.0f, 6.0f);
        DrawTrackPolyline(preview.points, proj, road, Fade(WHITE, 0.14f));
        DrawTrackPolyline(preview.points, proj, 1.5f, Fade(WHITE, selected ? 0.60f : 0.40f));
        DrawCircleV(proj.Apply(preview.points[0]), 3.5f, Color{88, 214, 104, 255}); // point de depart
    }

    float textTop = inset.y + inset.height;
    DrawTextCentered(def.name.c_str(), static_cast<int>(card.x + card.width * 0.5f),
                     static_cast<int>(textTop + 14.0f), selected ? 22 : 20, selected ? YELLOW : RAYWHITE);

    DrawTextWrapped(def.description.c_str(), static_cast<int>(card.x + 16.0f), static_cast<int>(textTop + 46.0f),
                    static_cast<int>(card.width - 32.0f), 15, 19, Fade(WHITE, 0.62f));

    if (def.surfaceStyle == SurfaceStyle::Abimee) {
        const Color amber{255, 178, 48, 255};
        const char* tag = "ROUTE ABIMEE";
        int tw = MeasureText(tag, 13);
        Rectangle badge{card.x + (card.width - static_cast<float>(tw) - 20.0f) * 0.5f, card.y + card.height - 40.0f,
                        static_cast<float>(tw) + 20.0f, 22.0f};
        DrawRectangleRounded(badge, 0.6f, 6, Fade(amber, 0.16f));
        DrawRectangleRoundedLinesEx(badge, 0.6f, 6, 1.0f, Fade(amber, 0.85f));
        DrawText(tag, static_cast<int>(badge.x + 10.0f), static_cast<int>(badge.y + 5.0f), 13, amber);
    }
}

} // namespace

// ---------------------------------------------------------------------------
// API publique
// ---------------------------------------------------------------------------

void DrawHud(const RaceState& race, int screenWidth, int screenHeight) {
    DrawHudEx(race, screenWidth, screenHeight, HudExtras{});
}

void DrawHudEx(const RaceState& race, int screenWidth, int screenHeight, const HudExtras& extras) {
    const std::vector<RacerEntry>& racers = race.Racers();
    const RacerEntry& player = racers[static_cast<size_t>(race.PlayerIndex())];

    DrawStandingsPanel(race, extras);
    DrawTimersPanel(race, extras, screenWidth);
    DrawSpeedGauge(player.car, screenHeight);
    DrawMinimap(race, extras, screenWidth, screenHeight);

    switch (race.Phase()) {
        case RacePhase::Countdown:
            DrawCountdown(race, screenWidth, screenHeight);
            break;
        case RacePhase::Racing:
            DrawGoFlash(race, screenWidth, screenHeight);
            break;
        case RacePhase::Finished:
            DrawFinishScreen(race, extras, screenWidth, screenHeight);
            break;
    }
}

void DrawMenu(const std::vector<TrackDef>& presets, int selectedIndex, int screenWidth, int screenHeight) {
    ClearBackground(Color{15, 17, 26, 255});
    DrawRectangleGradientV(0, 0, screenWidth, screenHeight, Color{22, 26, 40, 255}, Color{10, 11, 18, 255});

    // Titre avec ombre portee decalee.
    const char* title = "RACER";
    const int titleSize = 78;
    int cx = screenWidth / 2;
    int tw = MeasureText(title, titleSize);
    DrawText(title, cx - tw / 2 + 6, 46 + 6, titleSize, Color{110, 45, 16, 255});
    DrawText(title, cx - tw / 2, 46, titleSize, ORANGE);
    DrawLineEx(Vector2{static_cast<float>(cx - tw / 2), 134.0f}, Vector2{static_cast<float>(cx + tw / 2), 134.0f},
               3.0f, Fade(ORANGE, 0.55f));

    DrawTextCentered("Choisissez un circuit", cx, 152, 22, LIGHTGRAY);

    // Rangee de cartes centree ; la carte selectionnee est agrandie.
    int count = static_cast<int>(presets.size());
    if (count > 0) {
        const float gap = 20.0f;
        float cardW = std::min(250.0f, (static_cast<float>(screenWidth) - 60.0f -
                                        gap * static_cast<float>(count - 1)) / static_cast<float>(count));
        const float cardH = 280.0f;
        float totalW = cardW * static_cast<float>(count) + gap * static_cast<float>(count - 1);
        float x0 = (static_cast<float>(screenWidth) - totalW) * 0.5f;
        const float y0 = 246.0f;

        for (int i = 0; i < count; ++i) {
            Rectangle card{x0 + static_cast<float>(i) * (cardW + gap), y0, cardW, cardH};
            bool selected = (i == selectedIndex);
            if (selected) {
                // Leger zoom autour du centre de la carte.
                card.x -= 7.0f;
                card.y -= 7.0f;
                card.width += 14.0f;
                card.height += 14.0f;
            }
            DrawTrackCard(presets[static_cast<size_t>(i)], card, selected);
        }
    }

    DrawTextCentered("Haut/Bas : choisir   --   Entree : demarrer", cx, screenHeight - 56, 20, GRAY);
}

} // namespace racer
