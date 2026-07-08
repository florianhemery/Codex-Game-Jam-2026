/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Shared menu / driving input helpers for GameLoop
*/

#ifndef GAME_LOOP_INPUT_HPP_
#define GAME_LOOP_INPUT_HPP_

namespace racer {
namespace app {

bool keyJustPressed(int primary, int alternate = 0);
bool keyHeld(int primary, int alternate = 0);
bool charJustPressed(char lower, char upper);
bool menuConfirmPressed();
bool menuUpPressed();
bool menuDownPressed();
bool menuReturnPressed();
float readSteerTarget();

} // namespace app
} // namespace racer

#endif /* !GAME_LOOP_INPUT_HPP_ */
