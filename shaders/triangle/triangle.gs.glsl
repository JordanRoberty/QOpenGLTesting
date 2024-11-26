#version 150

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;
in vec4 v_Color[];

flat out vec4 g_Color[3];
out vec3 g_position;

void main()
{
  for(int i=0; i<3; ++i)
  {
    g_Color[i] = v_Color[i];
    g_position = vec3(0.0);
    g_position[i] = 1.0;
    gl_Position = gl_in[i].gl_Position;
    EmitVertex();
  }
  EndPrimitive();
}