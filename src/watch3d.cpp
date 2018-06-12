/*



.--,-``-.
/   /     '.      ,---,
/ ../        ;   .'  .' `\
\ ``\  .`-    ',---.'     \
\___\/   \   :|   |  .`\  |
   \   :   |:   : |  '  |
   /  /   / |   ' '  ;  :
   \  \   \ '   | ;  .  |
___ /   :   ||   | :  |  '
/   /\   /   :'   : | /  ;
/ ,,/  ',-    .|   | '` ,/
\ ''\        ; ;   :  .'
\   \     .'  |   ,.'
`--`-,,-'    '---'

3D RENDERING!


This is not the final API - I just pulled out the code from main.cpp.
Will deal with a cleaner version MAYBE later.
For the presentation we need something to show, so this will do.

Use the define in main.cpp to toggle between 2d (SDL only) and 3d rendering.
Have to find a way to open them at the same time (actually that is not
the problem but the windows events won't get handled  properly).



                             We staked out on a mission to find our inner peace
                             
                             Make it everlasting so nothing's incomplete
                             
                             It's easy being with you, sacred simplicity
                             
                             As long as we're together, there's no place I'd rather be
                             
                             
*/

#include "watch3d.h"
#include <fstream>

static Quad quad;
static std::vector<Quad> grid;
static GLuint vao;
static GLuint tex;
static GlChunkData gChunkData;

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
    gChunkData.chunks.resize(4 * 4);
    
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
    
	//glm::vec3 camForward = glm::normalize(camera.target - camera.pos);
    glm::mat4 View = glm::lookAt(camera.pos, camera.target, camera.up);
    
    // Model matrix : bottom left corner (tile 0,0) of grid is at 0,0 in world coordinates.
    // move -50 left and -50 down  so the tile 50, 50 is at 0, 0 in world coordinates.
    glm::mat4 Model = glm::mat4(
		1.0f, 0.0f, 0.0f, 0.0f, 
		0.0f, 1.0f, 0.0f, 0.0f, 
		0.0f, 0.0f, 1.0f, 0.0f, 
		-50.0f, 0.0f, -50.0f, 1.0f);
    glm::mat4 mvp = Projection * View * Model;
    
    return mvp;
}

void update_mvp(glm::mat4 & mvp, Shader & shader)
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

void create_chunk(Shader shader, W3dContext context, Chunk& chunk,
                  int x, int y)
{
    int quadCount = CHUNK_SIZE - 1;
    grid = std::vector<Quad>(quadCount * quadCount); // 4 quads per perlin-value
    for (int row = 0; row < quadCount; ++row)
    {
        for (int col = 0; col < quadCount; ++col)
        {
            
            // first triangle
            grid[row * quadCount + col].vertices[0] += col + chunk.x;
            grid[row * quadCount + col].vertices[1] += chunk.values[row * CHUNK_SIZE + col];
            grid[row * quadCount + col].vertices[2] += row + chunk.y;
            
            grid[row * quadCount + col].vertices[3] += col + chunk.x;
            grid[row * quadCount + col].vertices[4] += chunk.values[(row + 1) * CHUNK_SIZE+ col];
            grid[row * quadCount + col].vertices[5] += row + chunk.y;
            
            grid[row * quadCount + col].vertices[6] += col + chunk.x;
            grid[row * quadCount + col].vertices[7] += chunk.values[(row + 1) * CHUNK_SIZE + (col + 1)];
            grid[row * quadCount + col].vertices[8] += row + chunk.y;
            
			
            // second triangle
            grid[row * quadCount + col].vertices[9] += col + chunk.x;
            grid[row * quadCount + col].vertices[10] += chunk.values[(row + 1) * CHUNK_SIZE + (col + 1)];
            grid[row * quadCount + col].vertices[11] += row + chunk.y;
            
            grid[row * quadCount + col].vertices[12] += col + chunk.x;
            grid[row * quadCount + col].vertices[13] += chunk.values[row * CHUNK_SIZE + (col + 1)];
            grid[row * quadCount + col].vertices[14] += row + chunk.y;
            
            grid[row * quadCount + col].vertices[15] += col + chunk.x;
            grid[row * quadCount + col].vertices[16] += chunk.values[row * CHUNK_SIZE + col];
            grid[row * quadCount + col].vertices[17] += row + chunk.y;
        }
    }
    push_chunk(grid, x, y, 4); // TODO(Michael): magic value 4 for stride
}

void push_chunk(std::vector<Quad>& chunk, int row, int col, int stride)
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
    gChunkData.VAOs[row * stride + col] = vao;
    gChunkData.VBOs[row * stride + col] = vbo;
    gChunkData.chunks[row * stride + col] = chunk;
}

void render(W3dContext context, Shader shader)
{
    glUseProgram(shader.program);
    //glBindTexture(GL_TEXTURE_2D, tex);
    //glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, context.width, context.height, GL_RGBA,
    //                GL_UNSIGNED_BYTE, &image[0]);
    
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    for (int i = 0; i < gChunkData.chunks.size(); ++i)
    {
        glBindVertexArray(gChunkData.VAOs[i]);
        // glBindTexture(GL_TEXTURE_2D, sprite->texture.texture_id);
        int verticesPerQuad = 6;
        glDrawArrays(GL_TRIANGLES, 0, grid.size() * (verticesPerQuad));
    }
    
    //glPolygonMode(GL_FRONT, GL_LINE);
    SDL_GL_SwapWindow(context.sdlWnd);
}

// renders first column only
void renderToPGM(std::vector<Chunk>& chunks, std::string const & filename)
{
    int stride = 4;
    std::ofstream out(filename);
    if (!out)
        return;
    out << "P2" << "\n";
    out << CHUNK_SIZE << " " << CHUNK_SIZE * stride << "\n";
    out << "255" << "\n";
    for (int i = 0; i < stride; ++i)
    {
        for (int row = 0; row < CHUNK_SIZE; ++row)
        {
            for (int col = 0; col < CHUNK_SIZE; ++col)
            {
                int value = chunks[i * stride].values[row * CHUNK_SIZE + col] * 255;
                out << value << " ";
            }
            out << "\n";
        }
    }
    
    out.close();
}
