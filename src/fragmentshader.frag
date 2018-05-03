#version 150

in vec3 to_frag;
out vec4 frag_colour;
uniform sampler2D tex;

void main () 
{
  //frag_colour = vec4(to_frag, 1.0);
  frag_colour = texture2D(tex, to_frag.xy);
}
