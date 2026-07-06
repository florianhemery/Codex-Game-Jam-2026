#include "render/hud.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "raylib.h"

namespace racer {

namespace {

void FormatOrdinal(int position, char* buf, size_t bufSize) {
    if (position == 1) {
        std::snprintf(buf, bufSize, "1er");
    } else {
        std::snprintf(buf, bufSize, "%de", position);
    }
}

void FormatTime(float seconds, char* buf, size_t bufSize) {
    int m = static_cast<int>(seconds) / 60;
    float s = seconds - static_cast<float>(m * 60);
    std::snprintf(buf, bufSize, "%d:%05.2f", m, s);
}

} // namespace

void DrawHud(const RaceState& race, int screenWidth, int screenHeight) {
    const RacerEntry& player = race.Racers()[static_cast<size_t>(race.PlayerIndex())];

    if (race.Phase() == RacePhase::Countdown) {
        int n = static_cast<int>(std::ceil(race.CountdownRemaining()));
        char buf[8];
        std::snprintf(buf, sizeof(buf), n > 0 ? "%d" : "GO !", n);
        int fontSize = 96;
        int textWidth = MeasureText(buf, fontSize);
        DrawText(buf, screenWidth / 2 - textWidth / 2, screenHeight / 2 - fontSize / 2, fontSize, YELLOW);
    }

    char speedBuf[64];
    std::snprintf(speedBuf, sizeof(speedBuf), "%.0f km/h", std::fabs(player.car.speed) * 6.0f);
    DrawText(speedBuf, 20, screenHeight - 60, 32, WHITE);

    char lapBuf[64];
    std::snprintf(lapBuf, sizeof(lapBuf), "Tour %d/%d", std::min(player.lap + 1, race.LapsToWin()), race.LapsToWin());
    DrawText(lapBuf, 20, 20, 28, WHITE);

    char posBuf[16];
    FormatOrdinal(race.PlayerPosition(), posBuf, sizeof(posBuf));
    char posLine[64];
    std::snprintf(posLine, sizeof(posLine), "Position: %s / %d", posBuf, static_cast<int>(race.Racers().size()));
    DrawText(posLine, 20, 55, 28, WHITE);

    char timeBuf[32];
    FormatTime(race.ElapsedTime(), timeBuf, sizeof(timeBuf));
    DrawText(timeBuf, screenWidth - 160, 20, 28, WHITE);

    if (player.car.nitroRemaining < player.car.tuning.nitroCapacity) {
        float ratio = player.car.nitroRemaining / player.car.tuning.nitroCapacity;
        DrawRectangle(20, screenHeight - 90, 200, 16, Fade(BLACK, 0.5f));
        DrawRectangle(20, screenHeight - 90, static_cast<int>(200 * ratio), 16, ORANGE);
        DrawText("NITRO", 20, screenHeight - 110, 16, ORANGE);
    }

    if (race.Phase() == RacePhase::Finished) {
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.6f));

        char resultBuf[64];
        std::snprintf(resultBuf, sizeof(resultBuf), "ARRIVEE -- %s place", posBuf);
        int fontSize = 56;
        int textWidth = MeasureText(resultBuf, fontSize);
        DrawText(resultBuf, screenWidth / 2 - textWidth / 2, screenHeight / 2 - 100, fontSize, YELLOW);

        char finalTimeBuf[64];
        std::snprintf(finalTimeBuf, sizeof(finalTimeBuf), "Temps : %s", timeBuf);
        int tw2 = MeasureText(finalTimeBuf, 28);
        DrawText(finalTimeBuf, screenWidth / 2 - tw2 / 2, screenHeight / 2 - 30, 28, WHITE);

        const char* restart = "R pour recommencer";
        int tw3 = MeasureText(restart, 20);
        DrawText(restart, screenWidth / 2 - tw3 / 2, screenHeight / 2 + 20, 20, LIGHTGRAY);
    }
}

} // namespace racer
