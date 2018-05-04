#version 150

in vec3 vertex_pos;
in vec3 colors;
uniform sampler2D tex;
out vec3 fragColors;

void main ()
{
  //gl_TexCoord[0].xy = gl_MultiTexCoord0.xy;

  vec4 dv = texture2D(tex, colors.xy);
  fragColors = vec3(colors);
  gl_Position = vec4(vertex_pos.xy, vertex_pos.z + dv.x * 2.0f, 1.0);
}
