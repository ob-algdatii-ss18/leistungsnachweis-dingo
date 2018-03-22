#include <functional>
#include <random>
#include "TypeDef.h"

// Do not ever define this somewhere else, read documentation in the header for more info
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// C++11 Randoms http://www.cplusplus.com/reference/random/
static std::default_random_engine generator;
static std::uniform_int_distribution<s32> distribution(0, 255);
static auto u8UniformRandom = std::bind(distribution, generator);

int main()
{
    const int width = 640;
    const int height = 480;
    const int components = 4;  // RGBA

    // Generate Image Data. Each Component is a byte in RGBA order.
    std::vector<u8> image;
    image.reserve(width * height * components);
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            for (int c = 0; c < components; ++c)
            {
                image.push_back(u8UniformRandom());
            }
        }
    }

    // Write out image via lib
    stbi_write_png("Test.png", width, height, components, image.data(), width * components);

    return 0;
}
