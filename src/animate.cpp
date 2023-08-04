/*
    MIT License

    Copyright (c) 2023 Don Cross <cosinekitty@gmail.com>

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include <cstdio>
#include <cmath>
#include "raylib.h"
#include "lightning.hpp"

static Vector3 Vantage(float viewAngle);
static void Render(const Sapphire::LightningBolt& bolt);

int main(int argc, const char *argv[])
{
    const int screenWidth  = 900;
    const int screenHeight = 900;

    InitWindow(screenWidth, screenHeight, "Lightning simulation by Don Cross");
    Camera3D camera{};
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 50.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    SetTargetFPS(60);

    Sapphire::LightningBolt bolt(2000);
    bolt.generate();

    float viewAngle = 0.0f;

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_R))
            bolt.generate();

        viewAngle = std::fmod(viewAngle + 0.002f, 2.0 * M_PI);
        camera.position = Vantage(viewAngle);
        SetCameraMode(camera, CAMERA_FREE);
        UpdateCamera(&camera);
        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(camera);
        DrawGrid(10, 1.0f);
        Render(bolt);
        EndMode3D();
        EndDrawing();
    }
    CloseWindow();
    return 0;
}


static Vector3 Vantage(float viewAngle)
{
    Vector3 vec;

    const float radius = 15.0f;
    vec.x = radius * std::sin(viewAngle);
    vec.y = 6.0f;
    vec.z = radius * std::cos(viewAngle);

    return vec;
}


static void Render(const Sapphire::LightningBolt& bolt)
{
    const float scale = 10.0 / 4000.0;      // world-units per meter
    Vector3 startPos, endPos;
    Color color = PURPLE;

    for (const Sapphire::BoltSegment& seg : bolt.segments())
    {
        // Convert my coordinates with the x-y plane as horizontal,
        // to raylib coordinates with x-z plane as horizontal.
        // Preserve the right-hand rule.
        // Convert meters to world-units.

        startPos.x = scale * seg.a.x;
        startPos.y = scale * seg.a.z;
        startPos.z = -scale * seg.a.y;

        endPos.x = scale * seg.b.x;
        endPos.y = scale * seg.b.z;
        endPos.z = -scale * seg.b.y;

        DrawLine3D(startPos, endPos, color);
    }
}
