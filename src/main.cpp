#include <SDL.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <functional>
#include <numeric>
#include <random>
#include <string>
#include "Area.h"
#include "Chunk.h"
#include "Perlin.h"
#include "TypeDef.h"
#include "imgui_impl_sdl_gl3.h"
#include "watch3d.h"

unsigned int keystates[512];
int mouse_dx = 0;
int mouse_dy = 0;
void process_keys(Camera &camera)
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
    if (SDL_GetMouseState(0, 0) & SDL_BUTTON(SDL_BUTTON_RIGHT))
    {
        yaw += (DEG_TO_RAD(.5f)) * camera.velocity * mouse_dx * 0.1f;
        pitch -= (DEG_TO_RAD(.5f)) * camera.velocity * mouse_dy * 0.1f;
        mouse_dx = 0;
        mouse_dy = 0;
    }

    glm::vec3 front;
    front.x = cos(pitch) * cos(yaw);
    front.y = sin(pitch);
    front.z = cos(pitch) * sin(yaw);
    camera.target = camera.pos + glm::normalize(front);
}

Area *GetAreaForCoords(int x, int y, Area *def)
{
    if (x < 0 || x > 1)
        return def;
    if (y < 0 || y > 1)
        return def;

    int areaIndex = x + y * AREA_STRIDE;
    if (areaIndex >= 4)
        return def;

    return &g_areas[areaIndex];
}

static void CleanOGL()
{
    gChunkData.currentIndex = 0;
    gChunkData.chunks.clear();
    gChunkData.col.clear();
    gChunkData.chunks.resize(CHUNK_STRIDE * CHUNK_STRIDE * AREA_STRIDE * AREA_STRIDE);
    gChunkData.col.resize(CHUNK_STRIDE * CHUNK_STRIDE * AREA_STRIDE * AREA_STRIDE);

    for (int i = 0; i < CHUNK_STRIDE * CHUNK_STRIDE * AREA_STRIDE * AREA_STRIDE; ++i)
    {
        glDeleteVertexArrays(1, &gChunkData.VAOs[i]);
        glDeleteBuffers(1, &gChunkData.VAOs[i]);
    }
}

static void CalculateAllChunks(Shader &shader, W3dContext &renderCtx)
{
    std::vector<Chunk> chunks;
    for (int areaY = 0; areaY < AREA_STRIDE; ++areaY)
    {
        for (int areaX = 0; areaX < AREA_STRIDE; ++areaX)
        {
            for (u8 row = 0; row < CHUNK_STRIDE; ++row)
            {
                for (u8 col = 0; col < CHUNK_STRIDE; ++col)
                {
                    // Build proper area vector
                    std::vector<Area *> aVec;
                    int areaIndex = areaX + areaY * AREA_STRIDE;
                    Area *def = &g_areas[areaIndex];

                    Area *left = GetAreaForCoords(areaX - 1, areaY, def);
                    Area *top = GetAreaForCoords(areaX, areaY - 1, def);
                    Area *topleft = GetAreaForCoords(areaX - 1, areaY - 1, def);
                    Area *topright = GetAreaForCoords(areaX + 1, areaY - 1, def);
                    Area *right = GetAreaForCoords(areaX + 1, areaY, def);
                    Area *bottomright = GetAreaForCoords(areaX + 1, areaY + 1, def);
                    Area *bottom = GetAreaForCoords(areaX, areaY + 1, def);
                    Area *bottomleft = GetAreaForCoords(areaX - 1, areaY + 1, def);

                    int index = col + row * CHUNK_STRIDE;
                    ChunkType type = ChunkType::Inner;
                    switch (index)
                    {
                        case 0:
                            type = CornerTopLeft;
                            aVec.push_back(topleft);
                            aVec.push_back(top);
                            aVec.push_back(left);
                            aVec.push_back(def);
                            break;
                        case 1:
                            type = VerticalTop;
                            aVec.push_back(top);
                            aVec.push_back(def);
                            break;
                        case 2:
                            aVec.push_back(top);
                            aVec.push_back(topright);
                            aVec.push_back(def);
                            aVec.push_back(right);
                            type = CornerTopRight;
                            break;
                        case 3:
                            type = HorizontalLeft;
                            aVec.push_back(left);
                            aVec.push_back(def);
                            break;
                        case 4:
                            type = Inner;
                            aVec.push_back(def);
                            break;
                        case 5:
                            type = HorizontalRight;
                            aVec.push_back(def);
                            aVec.push_back(right);
                            break;
                        case 6:
                            type = CornerBottomLeft;
                            aVec.push_back(left);
                            aVec.push_back(def);
                            aVec.push_back(bottomleft);
                            aVec.push_back(bottom);
                            break;
                        case 7:
                            type = VerticalBottom;
                            aVec.push_back(def);
                            aVec.push_back(bottom);
                            break;
                        case 8:
                            type = CornerBottomRight;
                            aVec.push_back(def);
                            aVec.push_back(right);
                            aVec.push_back(bottom);
                            aVec.push_back(bottomright);
                            break;
                        default:
                            printf("test");
                            break;
                    }

                    /*std::vector<Area *> test;
                    test.push_back(def);
                    Chunk chunk(col, row, test, Inner);*/

                    Chunk chunk(col, row, aVec, type, def);
                    chunk.calculate();
                    chunks.push_back(chunk);
                    chunk.renderToPGM("chunk" + std::to_string(areaIndex) + "_" +
                                      std::to_string(col) + std::to_string(row) + ".pgm");  // debug

                    create_chunk(shader, renderCtx, chunks[chunks.size() - 1]);
                }
            }
        }
    }

    max = -100000000.f;
    min = 100000000.f;
    for (auto &&area : g_areas)
    {
        max = glm::max(area.global_amplitude, max);
        min = glm::min(area.global_amplitude, min);
    }
}

int main(int, char *[])
{
    const int width = 1280;
    const int height = 720;
    const int components = 4;  // RGBA

    float dummy = 1.f;
    for (int row = 0; row < AREA_STRIDE; ++row)
    {
        for (int col = 0; col < AREA_STRIDE; ++col)
        {
            Area area(col + 1, row + 1);
            area.amplitude = 1.4f * dummy;
            area.global_amplitude = 1.f;
            area.frequency = 0.7f * dummy;
            g_areas.push_back(area);
            dummy *= 2.f;
        }
    }

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    W3dContext renderCtx = initGL(width, height);
    ImGui_ImplSdlGL3_Init(renderCtx.sdlWnd);
    Camera camera = create_camera();
    glm::mat4 MVP = create_mvp(renderCtx, camera);
    Shader shader = create_shader_program();

    CalculateAllChunks(shader, renderCtx);

    bool running = true;

    u32 start = SDL_GetTicks();
    while (running)
    {
        mouse_dx = 0;
        mouse_dy = 0;
        // Update Window Messages
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0)
        {
            ImGui_ImplSdlGL3_ProcessEvent(&event);
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
            if (event.type == SDL_MOUSEMOTION)
            {
                mouse_dx = event.motion.xrel;
                mouse_dy = event.motion.yrel;
                break;
            }
        }
        ImGui_ImplSdlGL3_NewFrame(renderCtx.sdlWnd);
        process_keys(camera);

        MVP = create_mvp(renderCtx, camera);
        update_mvp(MVP, shader);

        // Calculate Delta-Time to see runtime
        u32 current = SDL_GetTicks();
        u32 deltams = current - start;
        start = current;
        printf("Delta Time: %dms\n", deltams);

        // Rendering
        {
            static float sliderSpeed = 0.01f;
            ImGui::Begin("Parameters", NULL, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                        1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::DragFloat("Slider Speed", &sliderSpeed, 0.01f);
            static int areaIndex = 0;
            ImGui::DragInt("Select Area Index", &areaIndex, 1, 0, g_areas.size() - 1);
            ImGui::Separator();
            ImGui::Text("Selected Area Coordinates (x,y): %i,%i", g_areas[areaIndex].x,
                        g_areas[areaIndex].y);
            ImGui::DragFloat("Inner Amplitude", &g_areas[areaIndex].amplitude, sliderSpeed, 0.f);
            ImGui::DragFloat("Amplitude", &g_areas[areaIndex].global_amplitude, sliderSpeed, 0.f);
            ImGui::DragFloat("Frequency", &g_areas[areaIndex].frequency, sliderSpeed, 0.f);
            static int octaves = 0;
            octaves = g_areas[areaIndex].octaves;
            ImGui::DragInt("Octaves", &octaves, 2, 8);
            g_areas[areaIndex].octaves = octaves;
            ImGui::Separator();
            if (ImGui::Button("Recalculate everything"))
            {
                CleanOGL();
                CalculateAllChunks(shader, renderCtx);
            }
            ImGui::End();
        }
        render(renderCtx, shader);
        ImGui::Render();
        // render_area(activeArea, renderCtx, shader);
        SDL_GL_SwapWindow(renderCtx.sdlWnd);
    }

    return 0;
}
