/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Centralized, remappable gameplay key bindings
*/

#include "Engine/Input/InputBindings.hpp"

#include <cstdio>

#include "raylib.h"

namespace racer::engine::input {

namespace {

constexpr int kActionCount = static_cast<int>(Action::Count);

int indexOf(Action action)
{
    return static_cast<int>(action);
}

// raylib does not ship a KeyboardKey -> name lookup, so cover the keys any
// action in this game can reasonably be bound to and fall back to the raw
// key code otherwise.
const char *keyDisplayName(int key)
{
    if (key >= KEY_A && key <= KEY_Z) {
        static char letter[2] = {0, 0};
        letter[0] = static_cast<char>('A' + (key - KEY_A));
        return letter;
    }
    if (key >= KEY_ZERO && key <= KEY_NINE) {
        static char digit[2] = {0, 0};
        digit[0] = static_cast<char>('0' + (key - KEY_ZERO));
        return digit;
    }
    switch (key) {
    case KEY_SPACE:
        return "Espace";
    case KEY_LEFT_SHIFT:
        return "Maj gauche";
    case KEY_RIGHT_SHIFT:
        return "Maj droite";
    case KEY_LEFT_CONTROL:
        return "Ctrl gauche";
    case KEY_ESCAPE:
        return "Echap";
    case KEY_ENTER:
        return "Entree";
    case KEY_TAB:
        return "Tab";
    case KEY_UP:
        return "Haut";
    case KEY_DOWN:
        return "Bas";
    case KEY_LEFT:
        return "Gauche";
    case KEY_RIGHT:
        return "Droite";
    default:
        return nullptr;
    }
}

} // namespace

InputBindings::InputBindings()
{
    bindings_[indexOf(Action::Accelerate)] = Binding{{KEY_W, KEY_Z, KEY_UP}};
    bindings_[indexOf(Action::Brake)] = Binding{{KEY_S, KEY_DOWN, 0}};
    bindings_[indexOf(Action::SteerLeft)] = Binding{{KEY_A, KEY_Q, KEY_LEFT}};
    bindings_[indexOf(Action::SteerRight)] = Binding{{KEY_D, KEY_RIGHT, 0}};
    bindings_[indexOf(Action::Handbrake)] = Binding{{KEY_SPACE, 0, 0}};
    bindings_[indexOf(Action::Nitro)] = Binding{{KEY_LEFT_SHIFT, 0, 0}};
    bindings_[indexOf(Action::ToggleMap)] = Binding{{KEY_M, 0, 0}};
    bindings_[indexOf(Action::Menu)] = Binding{{KEY_ESCAPE, 0, 0}};
}

InputBindings &InputBindings::instance()
{
    static InputBindings s_instance;
    return s_instance;
}

bool InputBindings::isHeld(Action action) const
{
    const Binding &binding = bindings_[indexOf(action)];

    for (int key : binding.keys) {
        if (key != 0 && IsKeyDown(key)) {
            return true;
        }
    }
    return false;
}

bool InputBindings::isPressed(Action action) const
{
    const Binding &binding = bindings_[indexOf(action)];

    for (int key : binding.keys) {
        if (key != 0 && IsKeyPressed(key)) {
            return true;
        }
    }
    return false;
}

int InputBindings::primaryKey(Action action) const
{
    return bindings_[indexOf(action)].keys[0];
}

void InputBindings::rebindPrimary(Action action, int key)
{
    Binding &binding = bindings_[indexOf(action)];

    binding.keys[0] = key;
    binding.keys[1] = 0;
    binding.keys[2] = 0;
}

const char *InputBindings::actionName(Action action)
{
    switch (action) {
    case Action::Accelerate:
        return "Accelerer";
    case Action::Brake:
        return "Freiner";
    case Action::SteerLeft:
        return "Braquer gauche";
    case Action::SteerRight:
        return "Braquer droite";
    case Action::Handbrake:
        return "Frein a main";
    case Action::Nitro:
        return "Nitro";
    case Action::ToggleMap:
        return "Grande carte";
    case Action::Menu:
        return "Menu / pause";
    default:
        return "?";
    }
}

void InputBindings::beginCapture(Action action)
{
    selected_ = action;
    capturing_ = true;
    skipNextCapture_ = true;
}

void InputBindings::applyCapturedKey(int key)
{
    rebindPrimary(selected_, key);
}

void InputBindings::updateDebugRemap()
{
    if (IsKeyPressed(KEY_F2)) {
        menuOpen_ = !menuOpen_;
        capturing_ = false;
        return;
    }
    if (!menuOpen_) {
        return;
    }
    if (!capturing_) {
        for (int i = 0; i < kActionCount; ++i) {
            if (IsKeyPressed(KEY_ONE + i)) {
                beginCapture(static_cast<Action>(i));
                return;
            }
        }
        return;
    }
    // The number key that selected the action was pressed on this very
    // frame; GetKeyPressed() would immediately hand it back to us and
    // "rebind" the action to itself. Swallow one frame of the queue first.
    if (skipNextCapture_) {
        skipNextCapture_ = false;
        GetKeyPressed();
        return;
    }
    int key = GetKeyPressed();

    while (key != 0) {
        if (key != KEY_F2) {
            applyCapturedKey(key);
            capturing_ = false;
            break;
        }
        key = GetKeyPressed();
    }
}

bool InputBindings::remapMenuOpen() const
{
    return menuOpen_;
}

bool InputBindings::isCapturingKey() const
{
    return capturing_;
}

Action InputBindings::selectedAction() const
{
    return selected_;
}

void InputBindings::drawDebugRemapOverlay(int screenWidth, int screenHeight) const
{
    if (!menuOpen_) {
        return;
    }

    const int panelW = 360;
    const int panelH = 40 + kActionCount * 20 + 30;
    const int x = screenWidth / 2 - panelW / 2;
    const int y = screenHeight / 2 - panelH / 2;

    DrawRectangle(x, y, panelW, panelH, Fade(BLACK, 0.78f));
    DrawRectangleLines(x, y, panelW, panelH, RAYWHITE);
    DrawText("Remapping (F2 pour fermer)", x + 14, y + 10, 16, RAYWHITE);

    for (int i = 0; i < kActionCount; ++i) {
        Action action = static_cast<Action>(i);
        char line[96];
        const char *keyName = keyDisplayName(primaryKey(action));

        if (capturing_ && selected_ == action) {
            std::snprintf(line, sizeof(line), "%d. %s : ...", i + 1,
                actionName(action));
        } else if (keyName != nullptr) {
            std::snprintf(line, sizeof(line), "%d. %s : %s", i + 1,
                actionName(action), keyName);
        } else {
            std::snprintf(line, sizeof(line), "%d. %s : touche %d", i + 1,
                actionName(action), primaryKey(action));
        }
        Color color = (selected_ == action)
            ? (capturing_ ? YELLOW : SKYBLUE)
            : RAYWHITE;
        DrawText(line, x + 14, y + 36 + i * 20, 14, color);
    }
    if (capturing_) {
        DrawText("Appuyez sur une touche...", x + 14, y + panelH - 22, 14,
            YELLOW);
    } else {
        DrawText("Choisissez une action (1-8)", x + 14, y + panelH - 22, 14,
            Fade(RAYWHITE, 0.8f));
    }
}

} // namespace racer::engine::input
