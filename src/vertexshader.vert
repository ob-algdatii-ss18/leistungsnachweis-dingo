#version 150

in vec3 vertex_pos;
in vec3 colors;
uniform sampler2D tex;
uniform mat4 MVP;
out vec3 fragColors;
out float vertexY;
out vec3 vertex;

void main ()
{
  vertex = vertex_pos;
  //gl_TexCoord[0].xy = gl_MultiTexCoord0.xy;
  // TODO: 100.0f is magic value and depends on grid-size
  vec4 dv = texture2D(tex, vec2(1.0f / 100.0f * vertex_pos.x, 1.0f / 100.0f * vertex_pos.z));
  vertexY = vertex_pos.y + dv.x;
  fragColors = vec3(colors);
  gl_Position = MVP * vec4(vertex_pos.x, vertex_pos.y + dv.x * 50.0f, vertex_pos.z, 1.0);
}
