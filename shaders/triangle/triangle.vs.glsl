#version 140

out vec4 v_Color;
out vec3 v_position;

void main()
{
  gl_Position =  gl_Vertex;
  v_position = gl_Vertex.xyz;
  v_Color = gl_Color;
  gl_FrontColor = gl_Color;
}