#version 140

flat in vec4 g_Color[3];
in vec3 g_position;

out vec4 fragColor;


void main()
{
  int i = (g_position.x > g_position.y && g_position.x > g_position.z) ? 0 : (g_position.y > g_position.z) ? 1 : 2;

  fragColor = g_Color[i];

}