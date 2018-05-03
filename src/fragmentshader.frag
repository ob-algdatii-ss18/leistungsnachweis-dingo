#version 150

in vec3 to_frag;
out vec4 frag_colour;

void main () 
{
  frag_colour = vec4(to_frag, 1.0);
}
