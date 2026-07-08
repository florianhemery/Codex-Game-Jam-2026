/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Shared menu / driving input helpers for GameLoop
*/

#include "App/GameLoopInput.hpp"

#include "raylib.h"

namespace racer {
namespace app {

bool keyJustPressed(int primary, int alternate)
{
    if (IsKeyPressed(primary)) {
        return true;
    }
    return alternate != 0 && IsKeyPressed(alternate);
}

bool keyHeld(int primary, int alternate)
{
    if (IsKeyDown(primary)) {
        return true;
    }
    return alternate != 0 && IsKeyDown(alternate);
}

bool charJustPressed(char lower, char upper)
{
    int ch = GetCharPressed();

    while (ch > 0) {
        if (ch == lower || ch == upper) {
            return true;
        }
        ch = GetCharPressed();
    }
    return false;
}

bool menuConfirmPressed()
{
    return keyJustPressed(KEY_ENTER) || keyJustPressed(KEY_SPACE);
}

bool menuUpPressed()
{
    return keyJustPressed(KEY_UP) || keyJustPressed(KEY_W, KEY_Z);
}

bool menuDownPressed()
{
    return keyJustPressed(KEY_DOWN) || keyJustPressed(KEY_S);
}

bool menuReturnPressed()
{
    return keyJustPressed(KEY_M) || keyJustPressed(KEY_ESCAPE)
        || charJustPressed('m', 'M');
}

float readSteerTarget()
{
    if (keyHeld(KEY_A, KEY_Q) || keyHeld(KEY_LEFT)) {
        return 1.0f;
    }
    if (keyHeld(KEY_D) || keyHeld(KEY_RIGHT)) {
        return -1.0f;
    }
    return 0.0f;
}

} // namespace app
} // namespace racer
