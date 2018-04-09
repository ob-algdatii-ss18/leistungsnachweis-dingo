#include "TypeDef.h"
#include <functional>
#include <random>
#include <numeric>
#include <array>
#include <algorithm>
#include <SDL.h>
#include <chrono>

const float period = 256.0f;

static inline float perlinFade(float t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);         // 6t^5 - 15t^4 + 10t^3
}

static inline int getPerm(const std::array<int, 256>& p, int value)
{
    return p[value % 256];
}

static inline float grad(int hash, float x, float y, float z)
{
    switch (hash & 0xF)
    {
    case 0x0: return  x + y;
    case 0x1: return -x + y;
    case 0x2: return  x - y;
    case 0x3: return -x - y;
    case 0x4: return  x + z;
    case 0x5: return -x + z;
    case 0x6: return  x - z;
    case 0x7: return -x - z;
    case 0x8: return  y + z;
    case 0x9: return -y + z;
    case 0xA: return  y - z;
    case 0xB: return -y - z;
    case 0xC: return  y + x;
    case 0xD: return -y + z;
    case 0xE: return  y - x;
    case 0xF: return -y - z;
    default: return 0; // never happens
    }
}

static inline float lerp(float a, float b, float x) {
    return a + x * (b - a);
}

// http://flafla2.github.io/2014/08/09/perlinnoise.html
static float perlin3D(const std::array<int, 256>& p, float x, float y, float z)
{
    x = fmod(x, period);
    y = fmod(y, period);
    z = fmod(z, period);

    // Corrdinates in 
    int xi = static_cast<int>(x) & 255;
    int yi = static_cast<int>(y) & 255;
    int zi = static_cast<int>(z) & 255;
    float xf = x - static_cast<int>(x);
    float yf = y - static_cast<int>(y);
    float zf = z - static_cast<int>(z);

    float u = perlinFade(xf);
    float v = perlinFade(yf);
    float w = perlinFade(zf);

    int aaa = getPerm(p, getPerm(p, getPerm(p, xi) + yi) + zi);
    int aba = getPerm(p, getPerm(p, getPerm(p, xi) + yi + 1) + zi);
    int aab = getPerm(p, getPerm(p, getPerm(p, xi) + yi) + zi + 1);
    int abb = getPerm(p, getPerm(p, getPerm(p, xi) + yi + 1) + zi + 1);
    int baa = getPerm(p, getPerm(p, getPerm(p, xi + 1) + yi) + zi);
    int bba = getPerm(p, getPerm(p, getPerm(p, xi + 1) + yi + 1) + zi);
    int bab = getPerm(p, getPerm(p, getPerm(p, xi + 1) + yi) + zi + 1);
    int bbb = getPerm(p, getPerm(p, getPerm(p, xi + 1) + yi + 1) + zi + 1);

    float x1 = lerp(grad(aaa, xf, yf, zf), grad(baa, xf - 1, yf, zf), u);
    float x2 = lerp(grad(aba, xf, yf - 1, zf), grad(bba, xf - 1, yf - 1, zf), u);
    float y1 = lerp(x1, x2, v);

    x1 = lerp(grad(aab, xf, yf, zf - 1), grad(bab, xf - 1, yf, zf - 1), u);
    x2 = lerp(grad(abb, xf, yf - 1, zf - 1), grad(bbb, xf - 1, yf - 1, zf - 1), u);
    float y2 = lerp(x1, x2, v);

    return (lerp(y1, y2, w) + 1) / 2;
}

int main(int, char*[])
{
    const int width = 640;
    const int height = 480;
    const int components = 4;  // RGBA

    //The window we'll be rendering to
    SDL_Window* window = NULL;

    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 640, 480);

    // Create data buffer
    std::vector<u8> image;
    image.resize(width * height * components);

    // Perlin Random Initialize
    // C++11 Randoms http://www.cplusplus.com/reference/random/
    u32 seed = (u32)std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);

    std::array<int, 256> permutation{};
    // Fill with 0 to 255
    std::iota(permutation.begin(), permutation.end(), 1);
    // Shuffle
    std::shuffle(permutation.begin(), permutation.end(), generator);

    // Running var for animation
    float z = 0.f;
    bool  running = true;

    u32 start = SDL_GetTicks();
    while (running)
    {
        // Update Window Messages
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0)
        {
            if (event.type == SDL_QUIT)
                running = false;
        }

        // Calculate Delta-Time to see runtime
        u32 current = SDL_GetTicks();
        u32 deltams = current - start;
        start = current;
        printf("Delta Time: %dms\n", deltams);

        // Rendering
        // Generate Image Data. Each Component is a byte in RGBA order.
        size_t idx = 0;
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                float perlin = perlin3D(permutation, x*0.01f, y*0.01f, z);
                u8 colorValue = static_cast<u8>(floor(perlin * 256));
                for (int c = 0; c < components; ++c)
                {
                    image[idx++] = colorValue;
                }
            }
        }

        // Write to texture
        int w, h, pitch;
        void* pixels;
        SDL_QueryTexture(texture, NULL, NULL, &w, &h);

        SDL_LockTexture(texture, nullptr, &pixels, &pitch);
        SDL_memcpy(pixels, image.data(), image.size() * sizeof(u8));
        SDL_UnlockTexture(texture);

        // Render to screen
        SDL_RenderCopy(renderer, texture, 0, 0);
        SDL_RenderPresent(renderer);
        z += 0.01f;
        z = fmod(z, period);
    }

    return 0;
}
