varying vec4 v_color;
varying vec3 v_normal;
varying vec2 v_texcoord;

out vec4 fragColor;

#include "peeling.frag"

void main()
{
  vec4 color = v_color;
  peeling(color);
  fragColor = color;
}