/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Shared menu / driving input helpers for GameLoop
*/

#include "App/GameLoopInput.hpp"

#include "raylib.h"

#include "Engine/Input/InputBindings.hpp"

namespace racer {
namespace app {

using engine::input::Action;
using engine::input::InputBindings;

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
    const InputBindings &bindings = InputBindings::instance();

    if (bindings.isHeld(Action::SteerLeft)) {
        return 1.0f;
    }
    if (bindings.isHeld(Action::SteerRight)) {
        return -1.0f;
    }
    return 0.0f;
}

} // namespace app
} // namespace racer
