/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Standalone render pipeline demo entry point
*/

#include "Engine/Render/RenderDemoScene.hpp"

using render_demo::RenderDemoScene;

int main()
{
    RenderDemoScene::initDemoWindow();
    RenderDemoScene::runAmbianceDemo();
    RenderDemoScene::shutdownDemoWindow();
    return 0;
}
