#include <SDL.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <functional>
#include <numeric>
#include <random>
#include "TypeDef.h"

//GUI includes
#include "imgui.h"
#include "imgui-SFML.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/CircleShape.hpp>

#include <thread> 

#include "Area.h"
#include "Chunk.h"
#include "Perlin.h"
#include "watch3d.h"

unsigned int keystates[512];
float zAxis = 0;

void process_keys(Camera& camera)
{
    // TODO(Michael): move statics out
    static float pitch = 0.0f;
    static float yaw = -90.0f;

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
        yaw += (DEG_TO_RAD(.5f)) * camera.velocity;
    }
    if (keystates[SDL_SCANCODE_LEFT])
    {
        yaw -= (DEG_TO_RAD(.5f)) * camera.velocity;
    }
    if (keystates[SDL_SCANCODE_UP])
    {
        pitch += (DEG_TO_RAD(.5f)) * camera.velocity;
    }
    if (keystates[SDL_SCANCODE_DOWN])
    {
        pitch -= (DEG_TO_RAD(.5f)) * camera.velocity;
    }
    glm::vec3 front;
    front.x = cos(pitch) * cos(yaw);
    front.y = sin(pitch);
    front.z = cos(pitch) * sin(yaw);
    camera.target = camera.pos + glm::normalize(front);
}

void runImGUI() {
	sf::RenderWindow window(sf::VideoMode(640, 480), "");
	window.setVerticalSyncEnabled(true);
	ImGui::SFML::Init(window);

	// let's use char array as buffer, see next part
	// for instructions on using std::string with ImGui
	char windowTitle[255] = "Perlin Noise GUI";

	window.setTitle(windowTitle);
	window.resetGLStates(); // call it if you only draw ImGui. Otherwise not needed.
	sf::Clock deltaClock;
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			ImGui::SFML::ProcessEvent(event);

			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}

		ImGui::SFML::Update(window, deltaClock.restart());

		ImGui::Begin("parameters", NULL, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::PushItemWidth(500);
		ImGui::SliderFloat("z-axis", &zAxis, 0.0f, 5.f);


		ImGui::End(); // end window
		ImGui::SFML::Render(window);
		window.display();

	}
	ImGui::SFML::Shutdown();
}


int main(int, char* [])
{
    const int width = 640;
    const int height = 480;
    const int components = 4;  // RGBA

    // Create data buffer
    std::vector<u8> image;
    image.resize(width * height * components);

    Area area;
	Area *areas[] = { &area };

    std::vector<Chunk> chunks;
    for (u8 x = 0; x < 4; ++x)
    {
        for (u8 y = 0; y < 4; ++y)
        {
            Chunk chunk(x, y, areas, ChunkType::Inner);
            chunk.calculate();
            chunks.push_back(chunk);
        }
    }

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
    create_grid(100, shader, renderCtx, image,
                MVP);  // TODO(Michael), grid size has to match value in v-shader.

	//
	std::thread imGUI(runImGUI);

    // Running var for animation
    float z = 0.f;
    bool running = true;

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
        size_t idx = 0;
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                float perlin = octavePerlin(x, y, zAxis, area);

                // Get 0.0 - 1.0 value to 0 - 255
                u8 colorValue = static_cast<u8>(perlin_fastfloor(perlin * 256));
                for (int c = 0; c < components; ++c)
                {
                    image[idx++] = colorValue;
                }
            }
        }

        render(renderCtx, shader, image);

        z += 0.05f;
        // Wrap the value to not run into floating point issues
        z = fmod(z, 256.f);
    }
		
	imGUI.join();

    return 0;
}
