/**
*  @file    Perlin.cpp
*  @author  
*  @date    05 June 2018
*/

#include "Perlin.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <functional>
#include <numeric>
#include <random>
#include "TypeDef.h"

static inline float perlinFade(float t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);  // 6t^5 - 15t^4 + 10t^3
}

static inline float grad(int hash, float x, float y, float z)
{
    switch (hash & 0xF)
    {
        case 0x0:
            return x + y;
        case 0x1:
            return -x + y;
        case 0x2:
            return x - y;
        case 0x3:
            return -x - y;
        case 0x4:
            return x + z;
        case 0x5:
            return -x + z;
        case 0x6:
            return x - z;
        case 0x7:
            return -x - z;
        case 0x8:
            return y + z;
        case 0x9:
            return -y + z;
        case 0xA:
            return y - z;
        case 0xB:
            return -y - z;
        case 0xC:
            return y + x;
        case 0xD:
            return -y + z;
        case 0xE:
            return y - x;
        case 0xF:
            return -y - z;
        default:
            return 0;  // never happens
    }
}

static inline float lerp(float a, float b, float x) { return a + x * (b - a); }

int perlin_fastfloor(float a)
{
    int ai = (int)a;
    return (a < ai) ? ai - 1 : ai;
}

// http://flafla2.github.io/2014/08/09/perlinnoise.html
static float perlin3D(const std::array<int, 512>& p, float x, float y, float z)
{
    int px = perlin_fastfloor(x);
    int py = perlin_fastfloor(y);
    int pz = perlin_fastfloor(z);

    int x0 = px & 255;
    int x1 = (px + 1) & 255;
    int y0 = py & 255;
    int y1 = (py + 1) & 255;
    int z0 = pz & 255;
    int z1 = (pz + 1) & 255;

    x -= px;
    y -= py;
    z -= pz;

    float u = perlinFade(x);
    float v = perlinFade(y);
    float w = perlinFade(z);

    // Randomization via 3x TableLookup
    int r0 = p[x0];
    int r1 = p[x1];

    int r00 = p[r0 + y0];
    int r01 = p[r0 + y1];
    int r10 = p[r1 + y0];
    int r11 = p[r1 + y1];

    int aaa = p[r00 + z0];
    int aba = p[r01 + z0];
    int aab = p[r00 + z1];
    int abb = p[r01 + z1];
    int baa = p[r10 + z0];
    int bba = p[r11 + z0];
    int bab = p[r10 + z1];
    int bbb = p[r11 + z1];

    // Calculate Gradient and lerp in x and y
    float gx1 = lerp(grad(aaa, x, y, z), grad(baa, x - 1, y, z), u);
    float gx2 = lerp(grad(aba, x, y - 1, z), grad(bba, x - 1, y - 1, z), u);
    float gy1 = lerp(gx1, gx2, v);

    gx1 = lerp(grad(aab, x, y, z - 1), grad(bab, x - 1, y, z - 1), u);
    gx2 = lerp(grad(abb, x, y - 1, z - 1), grad(bbb, x - 1, y - 1, z - 1), u);
    float gy2 = lerp(gx1, gx2, v);

    return (lerp(gy1, gy2, w) + 1) / 2;
}

float octavePerlin(float x, float y, float z, const Area& area)
{
    float total = 0.f;
    float frequency = area.frequency;
    float amplitude = area.amplitude;

    // Used for normalizing result to 0.0 - 1.0
    float maxValue = 0.f;

    for (int i = 0; i < area.octaves; i++)
    {
        total +=
            perlin3D(area.permutation, x * 0.01f * frequency, y * 0.01f * frequency, z) * amplitude;

        maxValue += amplitude;

        amplitude *= 0.5f;
        frequency *= 2.f;
    }

    return total / maxValue;
}