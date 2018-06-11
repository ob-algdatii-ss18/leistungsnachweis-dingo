#version 150

in vec3 fragColors;
out vec4 frag_color;
in float vertexY;
in vec3 vertex;
uniform sampler2D tex;

void main () 
{
  // vec4 colorValue = texture2D(tex, vec2(1.0f / 100.0f * vertex.x, 1.0f / 100.0f * vertex.z));

  // use z value as color
  frag_color = vec4(vec3(vertex.y), 1.0f);

  // use actual texture as color
  //frag_color = vec4(vec3(colorValue), 1.0f);
}
