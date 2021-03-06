#ifndef WATCH_3D_H
#define WATCH_3D_H

#include "TypeDef.h"

#include <stdio.h>
#include <vector>

#include <GL/glew.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "Chunk.h"

struct Camera
{
    glm::vec3 pos;
    glm::vec3 target;
    glm::vec3 up;
    float velocity;
};

struct W3dContext
{
    SDL_Window* sdlWnd;
    SDL_GLContext sdlGlCtx;
    int width, height;
};

struct Shader
{
    GLuint program;
};

struct Triangle  // CCW (OpenGL's default)
{
    GLfloat vertices[12] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f};
};

struct Quad  // this is CCW! FUCK AUTOFORMAT VS!!!! I AM INTENTIONALLY FORMATTING THE DATA THIS WAY.
             // FUCK YOU VS!
{
    GLfloat vertices[18] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
                            1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
};

struct GlChunkData
{
    int currentIndex = 0;
    GLuint VAOs[CHUNK_STRIDE * CHUNK_STRIDE * AREA_STRIDE * AREA_STRIDE];
    GLuint VBOs[CHUNK_STRIDE * CHUNK_STRIDE * AREA_STRIDE * AREA_STRIDE];
    std::vector<std::vector<Quad>> chunks;
    std::vector<int> col;
};

char* load_text(char const* filename);
void check_shader_error(GLuint shader);
W3dContext initGL(int width, int height);
Camera create_camera();
void update_mvp(glm::mat4& mvp, Shader& shader);
glm::mat4 create_mvp(W3dContext context, Camera camera);
Shader create_shader_program();
void create_chunk(Shader shader, W3dContext context, Chunk& chunk);
void push_chunk(std::vector<Quad>& chunk, Chunk& c);
void render(W3dContext context, Shader shader);
void render_area(Area& area, W3dContext context, Shader shader);

// helper
void renderToPGM(std::vector<Chunk>&, std::string const& filename);
extern GlChunkData gChunkData;
extern float max;
extern float min;
#endif
