#version 150

in vec2 texturePosForFrag;
uniform sampler2D tex;
out vec4 frag_colour;

void main () 
{
  frag_colour = texture2D(tex, texturePosForFrag);
}
