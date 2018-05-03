#version 150

in vec3 vertex_pos;
in vec3 colors;

out vec3 to_frag;

void main ()
{
  to_frag = colors;
  gl_Position = vec4(vertex_pos, 1.0);
}
