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
#include <cinttypes>
#include <mutex>

#include "raylib.h"
#include "wavefile.hpp"
#include "lightning.hpp"
#include "convolution.hpp"

#define RENDER_MODE_RAW 0
#define RENDER_MODE_CONVOLUTION 1

#define SELECTED_RENDER_MODE    RENDER_MODE_RAW

const int MAX_SAMPLES_PER_UPDATE = 4096;
const int SAMPLE_RATE = 44100;
const int NUM_CHANNELS = 2;

#if SELECTED_RENDER_MODE == RENDER_MODE_CONVOLUTION
static Sapphire::AudioBuffer ConvolutionAudio;
static bool LoadConvolutionAudio();
#endif

static void Render(const Sapphire::LightningBolt& bolt);
static void Save(const Sapphire::LightningBolt& bolt);
static void AudioInputCallback(void *buffer, unsigned frames);
static void MakeThunder(Sapphire::LightningBolt& bolt);

// Create a pair of ears for stereo audio output.
static const Sapphire::BoltPointList Listener
{
    Sapphire::BoltPoint{2500.0, +0.1, 0.0},
    Sapphire::BoltPoint{2500.0, -0.1, 0.0}
};

const std::size_t MAX_SEGMENTS = 2000;
static Sapphire::Thunder BackgroundThunder{Listener, MAX_SEGMENTS};

int main(int argc, const char *argv[])
{
    const int screenWidth  = 900;
    const int screenHeight = 900;

#if SELECTED_RENDER_MODE == RENDER_MODE_CONVOLUTION
    if (!LoadConvolutionAudio())
        return 1;
#endif

    InitWindow(screenWidth, screenHeight, "Lightning simulation by Don Cross");

    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(MAX_SAMPLES_PER_UPDATE);
    AudioStream stream = LoadAudioStream(SAMPLE_RATE, 16, NUM_CHANNELS);
    SetAudioStreamCallback(stream, AudioInputCallback);
    PlayAudioStream(stream);

    Camera3D camera{};
    camera.position = (Vector3){ 10.0f, 5.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 50.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    SetTargetFPS(60);

    Sapphire::LightningBolt bolt(MAX_SEGMENTS);
    MakeThunder(bolt);

    float viewAngle = 0.0f;

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_R))
            MakeThunder(bolt);

        if (IsKeyPressed(KEY_S))
            Save(bolt);

        viewAngle = std::fmod(viewAngle + 0.002f, 2.0 * M_PI);
        UpdateCamera(&camera, CAMERA_ORBITAL);
        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(camera);
        DrawGrid(10, 1.0f);
        Render(bolt);
        EndMode3D();
        EndDrawing();
    }

    UnloadAudioStream(stream);
    CloseAudioDevice();
    CloseWindow();
    return 0;
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

    // Draw an arrow pointing down to the left ear of the listener.
    // At rendering scale, the distance between the ears is negligible.
    Vector3 arrowBottom;
    arrowBottom.x = scale * Listener.at(0).x;
    arrowBottom.y = scale * Listener.at(0).z;
    arrowBottom.z = -scale * Listener.at(0).y;

    Vector3 arrowTop = arrowBottom;
    arrowTop.y += (scale * 100);

    Vector3 arrowSlant1 = arrowBottom;
    arrowSlant1.x += (scale * 30);
    arrowSlant1.y += (scale * 30);

    Vector3 arrowSlant2 = arrowBottom;
    arrowSlant2.x -= (scale * 30);
    arrowSlant2.y += (scale * 30);

    DrawLine3D(arrowBottom, arrowTop, GREEN);
    DrawLine3D(arrowBottom, arrowSlant1, GREEN);
    DrawLine3D(arrowBottom, arrowSlant2, GREEN);
}


static void Save(const Sapphire::LightningBolt& bolt)
{
    using namespace Sapphire;

    // Convert the lightning bolt into a thunder generator.
    Thunder thunder{Listener, bolt.getMaxSegments()};
    thunder.start(bolt);

    // Save lightning and thunder for study.
    FILE *outfile = fopen("output/thunder.txt", "wt");
    if (outfile != nullptr)
    {
        fprintf(outfile, "LightningBolt\n");
        for (const BoltSegment &b : bolt.segments())
            fprintf(outfile, "    (%lg, %lg, %lg) (%lg, %lg, %lg)\n", b.a.x, b.a.y, b.a.z, b.b.x, b.b.y, b.b.z);

        const std::size_t n = thunder.numEars();
        fprintf(outfile, "\nEar count = %d\n", static_cast<int>(n));

        for (std::size_t i = 0; i < n; ++i)
        {
            fprintf(outfile, "\nThunder[%d]\n", static_cast<int>(i));
            for (const ThunderSegment &t : thunder.segments(i))
                fprintf(outfile, "    (%lg, %lg)\n", t.distance1, t.distance2);
        }

        fclose(outfile);
    }
}


static std::vector<int16_t> AudioBuffer;
static std::size_t AudioBufferIndex;
static std::mutex AudioMutex;


static void AudioInputCallback(void *buffer, unsigned frames)
{
    using namespace std;

    int16_t *data = static_cast<int16_t *>(buffer);
    unsigned s = 0;

    {
        lock_guard<mutex> guard(AudioMutex);
        std::size_t n = AudioBuffer.size();
        for (unsigned i = 0; i < frames; ++i)
        {
            data[s++] = (AudioBufferIndex < n) ? AudioBuffer[AudioBufferIndex++] : 0;
            data[s++] = (AudioBufferIndex < n) ? AudioBuffer[AudioBufferIndex++] : 0;
        }
    }
}


static void MakeThunder(Sapphire::LightningBolt& bolt)
{
    using namespace std;

    bolt.generate();
    BackgroundThunder.start(bolt);
    Sapphire::AudioBuffer rawBuffer = BackgroundThunder.renderAudio(SAMPLE_RATE);

#if SELECTED_RENDER_MODE == RENDER_MODE_RAW
    const std::vector<float>& audioData = rawBuffer.buffer();
#elif SELECTED_RENDER_MODE == RENDER_MODE_CONVOLUTION
    printf("Starting convolution...\n");
    Sapphire::AudioBuffer audioBuffer = Sapphire::Convolution(rawBuffer, ConvolutionAudio);
    printf("Finished convolution.\n");
    const std::vector<float>& audioData = audioBuffer.buffer();
#else
    #error unknown render mode
#endif

    // Normalize the raw audio to fit within 16-bit integer samples.
    float maxSample = 0.0f;
    for (float x : audioData)
        maxSample = std::max(maxSample, std::abs(x));

    if (maxSample == 0.0f)
        maxSample = 1.0f;       // avoid division by zero

    vector<int16_t> audio;
    for (float x : audioData)
    {
        int16_t s = static_cast<int16_t>((x / maxSample) * 32700.0f);
        audio.push_back(s);
    }

    {
        lock_guard<mutex> guard(AudioMutex);
        AudioBuffer = audio;
        AudioBufferIndex = 0;
    }

    {
        // Now that we have started the new audio render, save the audio at our leisure.
        Sapphire::ScaledWaveFileWriter wave;
        const char *outWaveFileName = "output/thunder.wav";
        if (wave.Open(outWaveFileName, SAMPLE_RATE, NUM_CHANNELS))
        {
            wave.WriteSamples(audioData.data(), static_cast<int>(audioData.size()));
        }
        else
        {
            printf("ERROR: MakeThunder cannot open output file: %s\n", outWaveFileName);
        }
    }
}

#if SELECTED_RENDER_MODE == RENDER_MODE_CONVOLUTION
static bool LoadConvolutionAudio()
{
    const char *filename = "input/crash.wav";
    Sapphire::WaveFileReader reader;
    if (!reader.Open(filename))
    {
        printf("LoadConvolutionAudio: Cannot open input file: %s\n", filename);
        return false;
    }
    size_t nsamples = reader.TotalSamples();
    printf("LoadConvolutionAudio: file %s contains %lu samples, %d channels.\n", filename, static_cast<unsigned long>(nsamples), reader.Channels());
    std::vector<float> buffer = reader.Read(nsamples);
    if (buffer.size() != nsamples)
    {
        printf("LoadConvolutionAudio: read incorrect number of samples %lu\n", static_cast<unsigned long>(buffer.size()));
        return false;
    }
    ConvolutionAudio = Sapphire::AudioBuffer(buffer, reader.Channels());
    return true;
}
#endif
