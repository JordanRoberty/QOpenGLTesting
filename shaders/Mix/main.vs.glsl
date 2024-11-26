uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;
uniform int u_textureType;

varying vec3 v_color;
varying vec3 v_normal;
varying vec2 v_texcoord;
varying float is1DTexture;

void main()
{
    gl_Position = u_projection * u_view * u_model * gl_ModelViewMatrix * gl_Vertex;
    v_normal = gl_Normal;
    v_color = gl_Color.rgb;
    v_texcoord = gl_MultiTexCoord0.xy;
    is1DTexture = u_textureType == 0 ? 1.0 : 0.0;
}