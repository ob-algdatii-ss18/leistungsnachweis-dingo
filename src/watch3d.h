#ifndef WATCH_3D_H
#define WATCH_3D_H

#include "TypeDef.h"

#include <vector>
#include <stdio.h>



#define GLEW_STATIC
#include <GL/glew.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

struct Camera
{
	glm::vec3 pos;
	glm::vec3 target;
	glm::vec3 up;
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

struct Quad
{
	GLfloat vertices[18] = {
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f
	};
};

char* load_text(char const * filename);
void check_shader_error(GLuint shader);
W3dContext initGL(int width, int height);
Camera create_camera();
glm::mat4 create_mvp(W3dContext context, Camera camera);
Shader create_shader_program();
void create_grid(int gridSize, Shader shader,
	W3dContext context,
	std::vector<u8>& image,
	glm::mat4 mvp);
void render(W3dContext context, Shader shader, std::vector<u8>& image);

#endif