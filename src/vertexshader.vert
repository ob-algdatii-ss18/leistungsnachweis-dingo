#version 150

in vec3 vertex_pos;
in vec3 colors;


out vec3 to_frag;

void main ()
{
  //vec4 perlinValue = texture(tex, vec2(0.2, 0.456));
  //float height = 1.0f / perlinValue.x * 255; 
  to_frag = vertex_pos;
  gl_Position = vec4(vertex_pos.x, vertex_pos.y, vertex_pos.z, 1.0);
}
