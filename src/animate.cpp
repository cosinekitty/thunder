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

static Vector3 Vantage(float viewAngle);

int main(int argc, const char *argv[])
{
    const int screenWidth  = 900;
    const int screenHeight = 900;

    InitWindow(screenWidth, screenHeight, "Lightning/thunder simulation by Don Cross");
    Camera3D camera{};
    camera.position = (Vector3){10.0f, 5.0f, 10.0f};
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 50.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    SetTargetFPS(60);

    float viewAngle = 0.0f;

    for (int frame = 0; !WindowShouldClose(); ++frame)
    {
        viewAngle = std::fmod(viewAngle + 0.001f, 2.0 * M_PI);
        camera.position = Vantage(viewAngle);
        SetCameraMode(camera, CAMERA_FREE);

        if (frame % 100 == 0)
        {
            printf("camera pos(%f, %f, %f), target(%f, %f, %f), up(%f, %f, %f)\n",
                camera.position.x, camera.position.y, camera.position.z,
                camera.target.x, camera.target.y, camera.target.z,
                camera.up.x, camera.up.y, camera.up.z);
        }

        UpdateCamera(&camera);
        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(camera);
        DrawGrid(10, 1.0f);
        EndMode3D();
        EndDrawing();
    }
    CloseWindow();
    return 0;
}


static Vector3 Vantage(float viewAngle)
{
    // camera pos(-14.131022, 5.773502, 0.560571), target(0.000000, 0.000000, 0.000000), up(0.000000, 1.000000, 0.000000)
    // camera pos(-9.086400, 5.773502, 10.836852), target(0.000000, 0.000000, 0.000000), up(0.000000, 1.000000, 0.000000)
    // camera pos(1.912164, 5.773502, 14.012268), target(0.000000, 0.000000, 0.000000), up(0.000000, 1.000000, 0.000000)
    // camera pos(11.657681, 5.773502, 8.006153), target(0.000000, 0.000000, 0.000000), up(0.000000, 1.000000, 0.000000)
    // camera pos(13.764566, 5.773502, -3.246038), target(0.000000, 0.000000, 0.000000), up(0.000000, 1.000000, 0.000000)
    // camera pos(6.852185, 5.773502, -12.371241), target(0.000000, 0.000000, 0.000000), up(0.000000, 1.000000, 0.000000)
    // camera pos(-4.550715, 5.773502, -13.389960), target(0.000000, 0.000000, 0.000000), up(0.000000, 1.000000, 0.000000)
    // camera pos(-12.971193, 5.773502, -5.634551), target(0.000000, 0.000000, 0.000000), up(0.000000, 1.000000, 0.000000)
    // camera pos(-12.892283, 5.773502, 5.812835), target(0.000000, 0.000000, 0.000000), up(0.000000, 1.000000, 0.000000)
    // camera pos(-4.365895, 5.773502, 13.451357), target(0.000000, 0.000000, 0.000000), up(0.000000, 1.000000, 0.000000)
    // camera pos(7.023420, 5.773502, 12.274837), target(0.000000, 0.000000, 0.000000), up(0.000000, 1.000000, 0.000000)
    // camera pos(13.808298, 5.773502, 3.054655), target(0.000000, 0.000000, 0.000000), up(0.000000, 1.000000, 0.000000)
    // camera pos(11.545347, 5.773502, -8.167313), target(0.000000, 0.000000, 0.000000), up(0.000000, 1.000000, 0.000000)
    Vector3 vec;

    const float radius = 15.0f;
    vec.x = radius * std::cos(viewAngle);
    vec.y = 6.0f;
    vec.z = radius * std::sin(viewAngle);

    //vec.x = 10.0f;
    //vec.y = 5.0f;
    //vec.z = 10.0f;

    return vec;
}
