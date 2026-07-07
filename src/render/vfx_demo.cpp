// Demo autonome du systeme de particules : scenario time (fumee de drift le
// long d'une courbe, poussiere, nitro, etincelles, confettis, pluie) avec
// captures automatiques et fermeture auto. Aucun input requis.
#include <algorithm>
#include <cmath>
#include <cstdio>

#include "raylib.h"

#include "render/vfx.h"

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(960, 540, "vfx_demo -- particules");
    SetTargetFPS(60);

    // Camera fixe 3/4 sur la scene.
    Camera3D camera{};
    camera.position = Vector3{9.5f, 7.0f, 9.5f};
    camera.target = Vector3{0.0f, 1.0f, 0.0f};
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 55.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    racer::VfxSystem vfx;

    const float dt = 1.0f / 60.0f; // pas fixe : scenario deterministe
    int maxActive = 0;

    for (int frame = 0; frame <= 205 && !WindowShouldClose(); ++frame) {
        float tf = static_cast<float>(frame);

        // Salves regulieres de fumee de drift le long d'une trajectoire
        // courbe, avec de la poussiere hors piste juste a l'exterieur.
        if (frame % 2 == 0) {
            float ang = tf * dt * 1.2f;
            Vector3 pos{5.5f * std::cos(ang), 0.15f, 5.5f * std::sin(ang)};
            Vector3 vel{-std::sin(ang) * 6.6f, 0.0f, std::cos(ang) * 6.6f};
            vfx.EmitDriftSmoke(pos, vel);
            vfx.EmitOffroadDust(Vector3{pos.x * 1.15f, 0.1f, pos.z * 1.15f}, vel);
        }

        // ~f60 : rafale continue de nitro depuis un point, backDir tournant.
        if (frame >= 60) {
            float a = tf * 0.06f;
            Vector3 backDir{std::cos(a), 0.18f, std::sin(a)};
            float len = std::sqrt(backDir.x * backDir.x + backDir.y * backDir.y + backDir.z * backDir.z);
            vfx.EmitNitroFlame(Vector3{-3.5f, 0.7f, 2.5f},
                               Vector3{backDir.x / len, backDir.y / len, backDir.z / len},
                               Vector3{0.0f, 0.0f, 0.0f});
        }

        // ~f90 : etincelles contre un mur imaginaire + confettis au centre.
        if (frame >= 90 && frame <= 112 && frame % 4 == 0) {
            vfx.EmitSparks(Vector3{3.6f, 0.9f, -2.2f}, Vector3{-0.7f, 0.5f, 0.5f});
        }
        if (frame == 90 || frame == 100) {
            vfx.EmitConfetti(Vector3{0.0f, 2.6f, 0.0f});
        }

        // ~f120 : la pluie s'installe (montee lissee de l'intensite).
        if (frame == 120) vfx.SetRain(true);

        vfx.Update(dt, Vector3{0.0f, 0.0f, 0.0f});
        maxActive = std::max(maxActive, vfx.ActiveCount());

        BeginDrawing();
        ClearBackground(Color{36, 42, 54, 255});

        BeginMode3D(camera);
        DrawPlane(Vector3{0.0f, -0.02f, 0.0f}, Vector2{90.0f, 90.0f}, Color{47, 53, 62, 255});
        DrawGrid(24, 1.0f);
        // Reperes : bloc emetteur de nitro + mur des etincelles.
        DrawCube(Vector3{-3.5f, 0.35f, 2.5f}, 0.7f, 0.7f, 0.7f, Color{70, 76, 88, 255});
        DrawCube(Vector3{4.0f, 1.0f, -2.6f}, 0.25f, 2.0f, 2.6f, Color{62, 68, 80, 255});
        vfx.Draw(camera); // apres la scene opaque, dans BeginMode3D
        EndMode3D();

        DrawText(TextFormat("frame %d", frame), 12, 10, 20, RAYWHITE);
        DrawText(TextFormat("particules actives : %d / 4096", vfx.ActiveCount()), 12, 34, 20,
                 Color{255, 210, 90, 255});
        EndDrawing();

        if (frame == 45) TakeScreenshot("vfx_demo_0.png");
        if (frame == 85) TakeScreenshot("vfx_demo_1.png");
        if (frame == 115) TakeScreenshot("vfx_demo_2.png");
        if (frame == 200) TakeScreenshot("vfx_demo_3.png");
    }

    std::printf("[vfx_demo] max particules actives : %d (pool 4096)\n", maxActive);
    CloseWindow();
    return (maxActive < 4096) ? 0 : 1;
}
