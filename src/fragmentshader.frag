#version 150

in vec3 fragColors;
out vec4 frag_color;


void main () 
{
  frag_color = vec4(fragColors, 1.0f);
}
