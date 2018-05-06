#version 150

in vec3 vertex_pos;
in vec3 colors;
uniform sampler2D tex;
uniform mat4 MVP;
out vec3 fragColors;
out float vertexZ;
out vec3 vertex;

void main ()
{
  vertex = vertex_pos;
  //gl_TexCoord[0].xy = gl_MultiTexCoord0.xy;
  // TODO: 100.0f is magic value and depends on grid-size
  vec4 dv = texture2D(tex, vec2(1.0f / 100.0f * vertex_pos.x, 1.0f / 100.0f * vertex_pos.y));
  vertexZ = vertex_pos.z + dv.x;
  fragColors = vec3(colors);
  gl_Position = MVP * vec4(vertex_pos.xy, vertex_pos.z + dv.x * 30.0f, 1.0);
}
