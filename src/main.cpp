#include "TypeDef.h"
#include <functional>
#include <random>
#include <numeric>
#include <array>
#include <algorithm>
#include <SDL.h>
#include <chrono>

#include <stdio.h>

#define RENDER_OPEN_GL

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

static Camera camera;

static inline float perlinFade(float t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);         // 6t^5 - 15t^4 + 10t^3
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

char* load_text(char const * filename)
{
	FILE* f = fopen(filename, "rb");
	Assert(f);
	fseek(f, 0L, SEEK_END);
	long size = ftell(f);
	rewind(f);
	char* buffer = (char *)malloc(size + 1);
	fread(buffer, sizeof *buffer, size, f);
	buffer[size] = '\0';
	fclose(f);

	return buffer;
}

void check_shader_error(GLuint shader)
{
	GLint success = 0;
	GLint logSize = 0;
	GLchar buffer[255];

	if (glIsProgram(shader))
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
	}
	else
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	}

	if (success == GL_FALSE)
	{
		glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &logSize);
		glGetProgramInfoLog(shader, 255, &logSize, &buffer[0]);
		printf("Failed to Link Shader Program: %s\n", buffer);
	}
}

int main(int, char*[])
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

    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }


#ifdef RENDER_OPEN_GL

	SDL_Window* glWindow = NULL;
	SDL_GLContext glContext; // not sure if we actually need this handle...
	// gl context attributes
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	// SDL2 window for OpenGL context creation
	glWindow = SDL_CreateWindow(
		"OpenGL Window",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (glWindow == NULL)
	{
		printf("OpenGL Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return 1;
	}
	if ((glContext = SDL_GL_CreateContext(glWindow)) == NULL)
	{
		printf("Failed to bind Device Context to GL Render Context! SDL_Error: %s\n", 
			SDL_GetError());
		return 1;
	}

	// load GL extension pointers using GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		printf("Failed to get extended Render Context");
		return 1;
	}

	// glm garbage
	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);

	// Or, for an ortho camera :
	//glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates


	// Camera init and view-matrix
	camera.pos = glm::vec3(0, -150, 70);
	camera.target = glm::vec3(0, 0, -1.0);
	camera.up = glm::vec3(0, 1, 0);
	glm::mat4 View = glm::lookAt(
		camera.pos, // Camera is at (4,3,3), in World Space
		camera.target, // and looks at the origin
		camera.up  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// Model matrix : bottom left corner (tile 0,0) of grid is at 0,0 in world coordinates.
	// move -50 left and -50 down  so the tile 50, 50 is at 0, 0 in world coordinates.
	glm::mat4 Model = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
		                        0.0f, 1.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 1.0f, 0.0f,
								-50.0f, -50.0f, 0.0f, 1.0f);
	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 mvp = Projection * View * Model; // Remember, matrix multiplication is the other way around

	// create shaders
	// load shader text from files
	char * vertCode = load_text("../src/vertexshader.vert");
	char * fragCode = load_text("../src/fragmentshader.frag");

	// compile shader program
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertCode, NULL);
	glCompileShader(vertexShader);
	check_shader_error(vertexShader);

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragCode, NULL);
	glCompileShader(fragmentShader);
	check_shader_error(fragmentShader);

	// linking
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	// tell the shader what attribute belongs to which in variable name (OGL3.2 compatibility)
	// has to be done BEFORE linking!
	glBindAttribLocation(shaderProgram, 0, "vertex_pos");
	glBindAttribLocation(shaderProgram, 1, "colors");
	//glBindAttribLocation(shaderProgram, 1, "texture_pos");

	glLinkProgram(shaderProgram);
	check_shader_error(shaderProgram);

	glDetachShader(shaderProgram, vertexShader);
	glDetachShader(shaderProgram, fragmentShader);

	// test data
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

	Quad quad;

	int const gridSize = 100;
	Quad grid[gridSize * gridSize]; // TODO:(Michael): this can get dicy when big, cause on stack!
	//float z_ = 0.1f;
	//z_ = fmod(z_, 256.f);
	for (int row = 0; row < gridSize; ++row)
	{
		for (int col = 0; col < gridSize; ++col)
		{
			//float perlin = perlin3D(permutation, col*0.01f, row*0.01f, z_);
			// first triangle
			grid[row * gridSize + col].vertices[0] += col;
			grid[row * gridSize + col].vertices[1] += row;
			//grid[row * gridSize + col].vertices[2] = perlin;

			grid[row * gridSize + col].vertices[3] += col;
			grid[row * gridSize + col].vertices[4] += row;
			//grid[row * gridSize + col].vertices[5] = perlin;

			grid[row * gridSize + col].vertices[6] += col;
			grid[row * gridSize + col].vertices[7] += row;
			//grid[row * gridSize + col].vertices[8] = perlin;

			// second triangle
			grid[row * gridSize + col].vertices[9] += col;
			grid[row * gridSize + col].vertices[10] += row;
			//grid[row * gridSize + col].vertices[11] = perlin;

			grid[row * gridSize + col].vertices[12] += col;
			grid[row * gridSize + col].vertices[13] += row;
			//grid[row * gridSize + col].vertices[14] = perlin;

			grid[row * gridSize + col].vertices[15] += col;
			grid[row * gridSize + col].vertices[16] += row;
			//grid[row * gridSize + col].vertices[17] = perlin;
		}
	}

	GLfloat colors[] = {
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f
	};

	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo = 0;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(grid), grid[0].vertices, GL_STATIC_DRAW);
	// first param is index
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	// enable, affects only the previously bound VBOs!
	glEnableVertexAttribArray(0);

	GLuint color_vbo = 0;
	glGenBuffers(1, &color_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors , GL_STATIC_DRAW);
	// first param is index
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	// enable, affects only the previously bound VBOs!
	glEnableVertexAttribArray(1);

	glUseProgram(shaderProgram);
	// texture
	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		width,
		height,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		&image[0]
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	// link to tex uniform in vertex shader
	int tex_loc = glGetUniformLocation(shaderProgram, "tex");
	if (tex_loc == -1)
	{
		printf("no such active uniform variable in current shader program!\n");
	}
	glUniform1i(tex_loc, 0);
	glActiveTexture(GL_TEXTURE0);

	// uniform for MPV
	// Get a handle for our "MVP" uniform
	// Only during the initialisation
	GLuint MatrixID = glGetUniformLocation(shaderProgram, "MVP");

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);

	glViewport(0, 0, 640, 480);

#else

    //The window we'll be rendering to
    SDL_Window* window = NULL;
	// SDL2 window
    window = SDL_CreateWindow(
		"SDL Window", 
		SDL_WINDOWPOS_UNDEFINED, 
		SDL_WINDOWPOS_UNDEFINED, 
		width, height, 
		SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 640, 480);

#endif

    
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

#ifdef RENDER_OPEN_GL

			if (event.key.keysym.scancode == SDL_SCANCODE_D)
			{
				camera.pos += 1.0f * glm::normalize(glm::cross(camera.target, camera.up));
				// recompute matrices and upload to GPU
				glm::mat4 View = glm::lookAt(
					camera.pos, // Camera is at (4,3,3), in World Space
					camera.pos + camera.target, // target
					camera.up  // Head is up (set to 0,-1,0 to look upside-down)
				);

				//glm::mat4 Model = glm::mat4(1.0f);
				mvp = Projection * View * Model;
				glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
			}
#endif
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
                
                // Get 0.0 - 1.0 value to 0 - 255
                u8 colorValue = static_cast<u8>(perlin_fastfloor(perlin * 256));
                for (int c = 0; c < components; ++c)
                {
                    image[idx++] = colorValue;
                }
            }
        }


#ifdef RENDER_OPEN_GL

		glUseProgram(shaderProgram);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
		/* Clear our buffer with a red background */
		glClearColor(0.2, 0.2, 0.2, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		/* Swap our back buffer to the front */

		glBindVertexArray(vao);
		//glBindTexture(GL_TEXTURE_2D, sprite->texture.texture_id);
		int verticesPerQuad = 6;
		//glPolygonMode(GL_FRONT, GL_LINE);
		glDrawArrays(GL_LINES, 0, (sizeof grid / sizeof *grid) * verticesPerQuad); 
		SDL_GL_SwapWindow(glWindow);

#else 

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

#endif

        z += 0.05f;
        // Wrap the value to not run into floating point issues
        z = fmod(z, 256.f);
    }

    return 0;
}
