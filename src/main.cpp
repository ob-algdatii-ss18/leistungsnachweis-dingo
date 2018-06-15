#include <SDL.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <string>
#include <functional>
#include <numeric>
#include <random>
#include "TypeDef.h"
#include "Area.h"
#include "Chunk.h"
#include "Perlin.h"
#include "watch3d.h"

unsigned int keystates[512];

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

int main(int, char* [])
{
    const int width = 640;
    const int height = 480;
    const int components = 4;  // RGBA
    
    // Create data buffer
    std::vector<u8> image;
    image.resize(CHUNK_SIZE * CHUNK_SIZE * components);
    
    Area area;
	area.amplitude = 1.0f;
	area.frequency = 1.0f;
	Area *areas[] = { &area };
    
    std::vector<Chunk> chunks;
    for (u8 row = 0; row < CHUNK_STRIDE; ++row)
    {
        for (u8 col = 0; col < CHUNK_STRIDE; ++col)
        {
            Chunk chunk(col, row, areas, ChunkType::Inner);
            chunk.calculate();
            chunks.push_back(chunk);
			//chunk.renderToPGM("chunk" + std::to_string(col) + std::to_string(row) + ".pgm"); // debug
        }
    }
    
	renderToPGM(chunks, "chunks.pgm"); // debug
    
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
    
    // prepare chunk-data for render
    for (int row = 0; row < CHUNK_STRIDE; ++row)
    {
        for (int col = 0; col < CHUNK_STRIDE; ++col)
        {
            
            create_chunk(shader, renderCtx, chunks[row * CHUNK_STRIDE + col], row, col);
        }
    }
    
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
        
        render(renderCtx, shader);
        
    }
    
    return 0;
}
