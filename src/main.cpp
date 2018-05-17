#include <SDL.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <functional>
#include <numeric>
#include <random>
#include "TypeDef.h"

#include "watch3d.h"

unsigned int keystates[512];

void process_keys(Camera & camera)
{
	if (keystates[SDL_SCANCODE_A])
	{
		glm::vec3 camForward = glm::normalize((camera.target - camera.pos));
		glm::vec3 left = glm::normalize(glm::cross(-camForward, camera.up));
		camera.pos += left * camera.velocity;
		camera.target = camera.pos + camForward;
	}
	if (keystates[SDL_SCANCODE_D])
	{
		glm::vec3 camForward = glm::normalize((camera.target - camera.pos));
		glm::vec3 right = glm::normalize(glm::cross(camForward, camera.up));
		camera.pos += right * camera.velocity;
		camera.target = camera.pos + camForward;
	}
	if (keystates[SDL_SCANCODE_W])
	{
		glm::vec3 camForward = glm::normalize((camera.target - camera.pos));
		camera.pos += camForward * camera.velocity;
		camera.target = camera.pos + camForward;
	}
	if (keystates[SDL_SCANCODE_S])
	{
		glm::vec3 camForward = glm::normalize((camera.target - camera.pos));
		camera.pos -= camForward * camera.velocity;
		camera.target = camera.pos + camForward;
	}
	if (keystates[SDL_SCANCODE_RIGHT])
	{
		float yaw = ((float)PI / 360.0f) * camera.velocity;
		glm::vec3 camForward = glm::normalize((camera.target - camera.pos));
		camForward.x = (camForward.x * cos(yaw) - camForward.z * sin(yaw));
		camForward.z = (camForward.x * sin(yaw) + camForward.z * cos(yaw));
		camera.target = camera.pos + camForward;
	}
	if (keystates[SDL_SCANCODE_LEFT])
	{
		float yaw = -((float)PI / 360.0f) * camera.velocity;
		glm::vec3 camForward = glm::normalize((camera.target - camera.pos));
		camForward.x = (camForward.x * cos(yaw) - camForward.z * sin(yaw));
		camForward.z = (camForward.x * sin(yaw) + camForward.z * cos(yaw));
		camera.target = camera.pos + camForward;
	}
	if (keystates[SDL_SCANCODE_UP])
	{
		float pitch = -((float)PI / 360.0f) * camera.velocity;
		glm::vec3 camForward = glm::normalize((camera.target - camera.pos));
		camForward.z = (camForward.z * cos(pitch) - camForward.y * sin(pitch));
		camForward.y = (camForward.z * sin(pitch) + camForward.y * cos(pitch));
		camera.target = camera.pos + camForward;
	}
	if (keystates[SDL_SCANCODE_DOWN])
	{
		float pitch = ((float)PI / 360.0f) * camera.velocity;
		glm::vec3 camForward = glm::normalize((camera.target - camera.pos));
		camForward.z = (camForward.z * cos(pitch) - camForward.y * sin(pitch));
		camForward.y = (camForward.z * sin(pitch) + camForward.y * cos(pitch));
		camera.target = camera.pos + camForward;
	}
}

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

static int perlin_fastfloor(float a)
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

float OctavePerlin(const std::array<int, 512>& permutation, float x, float y, float z, int octaves)
{
    float total = 0.f;
    float frequency = .4f;
    float amplitude = .6f;

    // Used for normalizing result to 0.0 - 1.0
    float maxValue = 0.f;

    for (int i = 0; i < octaves; i++)
    {
        total += perlin3D(permutation, x * 0.01f * frequency, y * 0.01f * frequency, z) * amplitude;

        maxValue += amplitude;

        amplitude *= 0.5f;
        frequency *= 2.f;
    }

    return total / maxValue;
}

int main(int, char* [])
{
    const int width = 640;
    const int height = 480;
    const int components = 4;  // RGBA

    // Create data buffer
    std::vector<u8> image;
    image.resize(width * height * components);

    // Perlin Random Initialize
    // C++11 Randoms http://www.cplusplus.com/reference/random/
    u32 seed = (u32)std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);

    std::array<int, 512> permutation{};
    // Fill with 0 to 255
    auto permEnd = permutation.begin() + 256;
    std::iota(permutation.begin(), permEnd, 1);
    // Shuffle
    std::shuffle(permutation.begin(), permEnd, generator);
    // Copy back to avoid overflows (we can query from [0,512[ to avoid doing modulos everywhere
    std::copy(permutation.begin(), permEnd, permEnd);

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    W3dContext renderCtx = initGL(width, height);
    Camera camera = create_camera();
    glm::mat4 MVP = create_mvp(renderCtx, camera);
    Shader shader = create_shader_program();
    create_grid(100, shader, renderCtx, image, MVP); // TODO(Michael), grid size has to match value in v-shader.

    // Running var for animation
    float z = 0.f;
    bool running = true;
	bool first = true;

    u32 start = SDL_GetTicks();
    while (running)
    {
        // Update Window Messages
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0)
        {
            if (event.type == SDL_QUIT)
                running = false;
			if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
				running = false;

			if (event.type == SDL_KEYDOWN)
			{
				keystates[event.key.keysym.scancode] = 1;
			}
			if (event.type == SDL_KEYUP)
			{
				keystates[event.key.keysym.scancode] = 0;
			}
        }
		process_keys(camera);

		MVP = create_mvp(renderCtx, camera);
		update_mvp(MVP, shader);

        // Calculate Delta-Time to see runtime
        u32 current = SDL_GetTicks();
        u32 deltams = current - start;
        start = current;
        printf("Delta Time: %dms\n", deltams);

        // Rendering
        // Generate Image Data. Each Component is a byte in RGBA order.
		if (first)
		{
			size_t idx = 0;
			for (int y = 0; y < height; ++y)
			{
				for (int x = 0; x < width; ++x)
				{
					float perlin = OctavePerlin(permutation, x, y, z, 6);

					// Get 0.0 - 1.0 value to 0 - 255
					u8 colorValue = static_cast<u8>(perlin_fastfloor(perlin * 256));
					for (int c = 0; c < components; ++c)
					{
						image[idx++] = colorValue;
					}
				}
			}
			first = false;
		}

        render(renderCtx, shader, image);

        z += 0.05f;
        // Wrap the value to not run into floating point issues
        z = fmod(z, 256.f);
    }


    return 0;
}
