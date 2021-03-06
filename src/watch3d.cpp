#include "watch3d.h"
#include <fstream>

static Quad quad;
static std::vector<Quad> grid;
static GLuint vao;
static GLuint tex;

GlChunkData gChunkData;
float max;
float min;

char* load_text(char const* filename)
{
    FILE* f = fopen(filename, "rb");
    Assert(f);
    fseek(f, 0L, SEEK_END);
    long size = ftell(f);
    rewind(f);
    char* buffer = (char*)malloc(size + 1);
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

W3dContext initGL(int width, int height)
{
    SDL_Window* glWindow = NULL;
    SDL_GLContext glContext;  // not sure if we actually need this handle...
    // gl context attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // SDL2 window for OpenGL context creation
    glWindow = SDL_CreateWindow("OpenGL Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (glWindow == NULL)
    {
        printf("OpenGL Window could not be created! SDL_Error: %s\n", SDL_GetError());
    }
    if ((glContext = SDL_GL_CreateContext(glWindow)) == NULL)
    {
        printf("Failed to bind Device Context to GL Render Context! SDL_Error: %s\n",
               SDL_GetError());
    }

    // load GL extension pointers using GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to get extended Render Context");
    }

    // init data
    // TODO(Michael): do this elsewhere...
    gChunkData.chunks.resize(CHUNK_STRIDE * CHUNK_STRIDE * AREA_STRIDE * AREA_STRIDE);
    gChunkData.col.resize(CHUNK_STRIDE * CHUNK_STRIDE * AREA_STRIDE * AREA_STRIDE);

    return {glWindow, glContext, width, height};
}

Camera create_camera()
{
    Camera camera;

    // Camera init
    camera.pos = glm::vec3(0.0f, 40.0f, 100.0f);
    camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
    camera.up = glm::vec3(0.0f, 1.0f, 0.0f);
    camera.velocity = 2.5f;

    return camera;
}

glm::mat4 create_mvp(W3dContext context, Camera camera)
{
    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 Projection = glm::perspective(
        glm::radians(45.0f), (float)context.width / (float)context.height, 0.1f, 1000.0f);

    // glm::vec3 camForward = glm::normalize(camera.target - camera.pos);
    glm::mat4 View = glm::lookAt(camera.pos, camera.target, camera.up);

    // Model matrix : bottom left corner (tile 0,0) of grid is at 0,0 in world coordinates.
    // move -50 left and -50 down  so the tile 50, 50 is at 0, 0 in world coordinates.
    glm::mat4 Model = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                                0.0f, -50.0f, 0.0f, -50.0f, 1.0f);
    glm::mat4 mvp = Projection * View * Model;

    return mvp;
}

void update_mvp(glm::mat4& mvp, Shader& shader)
{
    GLuint MatrixID = glGetUniformLocation(shader.program, "MVP");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
}

Shader create_shader_program()
{
    // create shaders
    // load shader text from files
    char* vertCode = load_text("../src/vertexshader.vert");
    char* fragCode = load_text("../src/fragmentshader.frag");

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
    // glBindAttribLocation(shaderProgram, 1, "texture_pos");

    glLinkProgram(shaderProgram);
    check_shader_error(shaderProgram);

    glDetachShader(shaderProgram, vertexShader);
    glDetachShader(shaderProgram, fragmentShader);

    return {shaderProgram};
}

void create_chunk(Shader shader, W3dContext context, Chunk& chunk)
{
    int quadCount = CHUNK_SIZE - 1;
    grid = std::vector<Quad>(quadCount * quadCount);
    int offsetX = chunk.x;
    int offsetY = chunk.y;
    int areaX = chunk.area->x * CHUNK_SIZE * 3;
    int areaY = chunk.area->y * CHUNK_SIZE * 3;
    for (int row = 0; row < quadCount; ++row)
    {
        for (int col = 0; col < quadCount; ++col)
        {
            // first triangle
            grid[row * quadCount + col].vertices[0] += col + offsetX + areaX;
            grid[row * quadCount + col].vertices[1] += chunk.values[row * CHUNK_SIZE + col];
            grid[row * quadCount + col].vertices[2] += row + offsetY + areaY;

            grid[row * quadCount + col].vertices[3] += col + offsetX + areaX;
            grid[row * quadCount + col].vertices[4] += chunk.values[(row + 1) * CHUNK_SIZE + col];
            grid[row * quadCount + col].vertices[5] += row + offsetY + areaY;

            grid[row * quadCount + col].vertices[6] += col + offsetX + areaX;
            grid[row * quadCount + col].vertices[7] +=
                chunk.values[(row + 1) * CHUNK_SIZE + (col + 1)];
            grid[row * quadCount + col].vertices[8] += row + offsetY + areaY;

            // second triangle
            grid[row * quadCount + col].vertices[9] += col + offsetX + areaX;
            grid[row * quadCount + col].vertices[10] +=
                chunk.values[(row + 1) * CHUNK_SIZE + (col + 1)];
            grid[row * quadCount + col].vertices[11] += row + offsetY + areaY;

            grid[row * quadCount + col].vertices[12] += col + offsetX + areaX;
            grid[row * quadCount + col].vertices[13] += chunk.values[row * CHUNK_SIZE + (col + 1)];
            grid[row * quadCount + col].vertices[14] += row + offsetY + areaY;

            grid[row * quadCount + col].vertices[15] += col + offsetX + areaX;
            grid[row * quadCount + col].vertices[16] += chunk.values[row * CHUNK_SIZE + col];
            grid[row * quadCount + col].vertices[17] += row + offsetY + areaY;
        }
    }
    push_chunk(grid, chunk);
}

void push_chunk(std::vector<Quad>& chunk, Chunk& c)
{
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(chunk[0]) * chunk.size(), &chunk[0], GL_STATIC_DRAW);
    // first param is index
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    // enable, affects only the previously bound VBOs!
    glEnableVertexAttribArray(0);

    gChunkData.VAOs[gChunkData.currentIndex] = vao;
    gChunkData.VBOs[gChunkData.currentIndex] = vbo;
    gChunkData.chunks[gChunkData.currentIndex] = chunk;
    if (c.type == Inner)
    {
        gChunkData.col[gChunkData.currentIndex] = 0;
    }

    if (c.type != ChunkType::Inner && c.type <= 4)
    {
        gChunkData.col[gChunkData.currentIndex] = 1;
    }
    if (c.type > 4)
    {
        gChunkData.col[gChunkData.currentIndex] = 2;
    }

    gChunkData.currentIndex++;
}

void render(W3dContext context, Shader shader)
{
    glEnable(GL_DEPTH_TEST);
    glUseProgram(shader.program);
    // glBindTexture(GL_TEXTURE_2D, tex);
    // glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, context.width, context.height, GL_RGBA,
    //                GL_UNSIGNED_BYTE, &image[0]);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    int i = 0;

    for (int areaIndexY = 0; areaIndexY < AREA_STRIDE; ++areaIndexY)
    {
        for (int areaIndexX = 0; areaIndexX < AREA_STRIDE; ++areaIndexX)
        {
            for (int row = 0; row < CHUNK_STRIDE; ++row)
            {
                for (int col = 0; col < CHUNK_STRIDE; ++col)
                {
                    glBindVertexArray(gChunkData.VAOs[i]);

                    GLuint maxI = glGetUniformLocation(shader.program, "maxAmp");
                    GLuint minI = glGetUniformLocation(shader.program, "minAmp");

                    glUniform1f(maxI, max);
                    glUniform1f(minI, min);
                    // glBindTexture(GL_TEXTURE_2D, sprite->texture.texture_id);
                    int verticesPerQuad = 6;
                    glDrawArrays(GL_TRIANGLES, 0, grid.size() * (verticesPerQuad));
                    i++;
                }
            }
        }
    }

    // glPolygonMode(GL_FRONT, GL_LINE);
}

void render_area(Area& area, W3dContext context, Shader shader) {}

// renders all chunks to pgm heightmap
void renderToPGM(std::vector<Chunk>& chunks, std::string const& filename)
{
    std::ofstream out(filename);
    if (!out)
        return;
    out << "P2"
        << "\n";
    out << CHUNK_SIZE * CHUNK_STRIDE << " " << CHUNK_SIZE * CHUNK_STRIDE << "\n";
    out << "255"
        << "\n";
    for (int i = 0; i < CHUNK_STRIDE; ++i)
    {
        for (int row = 0; row < CHUNK_SIZE; ++row)
        {
            for (int j = 0; j < CHUNK_STRIDE; ++j)
            {
                for (int col = 0; col < CHUNK_SIZE; ++col)
                {
                    int value = chunks[i * CHUNK_STRIDE + j].values[row * CHUNK_SIZE + col] * 255;
                    out << value << " ";
                }
                out << "\n";
            }
        }
    }
    out.close();
}
