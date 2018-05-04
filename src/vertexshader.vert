#version 150

in vec3 vertex_pos;
in vec3 colors;
uniform sampler2D tex;
out vec3 fragColors;

void main ()
{
  vec4 dv = texture2D(tex, vertex_pos.xy);
  fragColors = colors;
  gl_Position = vec4(vertex_pos, 1.0);
}
