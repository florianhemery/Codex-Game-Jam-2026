/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Centralized, remappable gameplay key bindings
*/

#ifndef INPUT_BINDINGS_HPP_
#define INPUT_BINDINGS_HPP_

namespace racer::engine::input {

// Logical gameplay actions. Keep this list focused on the inputs that were
// previously hardcoded IsKeyDown/IsKeyPressed(KEY_...) calls scattered across
// App/GameLoop*.cpp, so every one of them can be looked up / rebound here.
enum class Action {
    Accelerate,
    Brake,
    SteerLeft,
    SteerRight,
    Handbrake,
    Nitro,
    ToggleMap,
    Menu,
    Count
};

// Small, in-memory remappable key table. A save system may persist this in
// the future (see src/Save/); for now bindings live for the process lifetime
// only, which is enough to make remapping actually usable in a session.
class InputBindings {
public:
    static InputBindings &instance();

    bool isHeld(Action action) const;
    bool isPressed(Action action) const;

    int primaryKey(Action action) const;
    void rebindPrimary(Action action, int key);
    static const char *actionName(Action action);

    // Minimal but real runtime remap flow, driven entirely by hotkeys so it
    // does not require a dedicated menu screen:
    //   F2       -> open/close the remap selector
    //   1..8     -> while the selector is open, pick the action to rebind
    //   next key -> becomes the new binding for the selected action
    // Call once per frame from the active game loop (menu, open world and
    // race all share the same bindings/remap state).
    void updateDebugRemap();
    bool remapMenuOpen() const;
    bool isCapturingKey() const;
    Action selectedAction() const;

    // Draws a tiny raylib overlay describing the remap state. Safe to call
    // every frame; it no-ops unless the remap selector is open.
    void drawDebugRemapOverlay(int screenWidth, int screenHeight) const;

private:
    InputBindings();

    struct Binding {
        int keys[3] = {0, 0, 0};
    };

    void beginCapture(Action action);
    void applyCapturedKey(int key);

    Binding bindings_[static_cast<int>(Action::Count)];
    bool menuOpen_ = false;
    bool capturing_ = false;
    bool skipNextCapture_ = false;
    Action selected_ = Action::Accelerate;
};

} // namespace racer::engine::input

#endif /* !INPUT_BINDINGS_HPP_ */
