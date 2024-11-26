varying vec3 v_color;
varying vec3 v_normal;
varying vec2 v_texcoord;
varying float is1DTexture;

out vec4 fragColor;

uniform int u_useDepthPeeling;
uniform bool u_hasTexture;
uniform sampler1D u_texture1D;
uniform sampler2D u_texture2D;


#include "peeling.frag"

vec4 basicColor()
{
  vec4 color = vec4(1.0, 1.0, 1.0, 1.0);

  if(!u_hasTexture)
  {
    if(v_color != vec3(0.0, 0.0, 0.0))
    {
     color = vec4(v_color, 1.0);
    }
    else
    {
     color = vec4(v_normal, 1.0);
    }
  }
  else if(is1DTexture == 1.0f)
  {
    color = texture(u_texture1D, v_texcoord.x);
    //color = vec4(0.0, 0.0, 1.0, 1.0); //blue
  }
  else
  {
    color = texture(u_texture2D, v_texcoord);
    color = vec4(1.0, 0.0, 0.0, 1.0); //red
  }

  color = vec4(v_normal, 0.5);
  color.a = 0.5;
  return color;
}

void main()
{
  vec4 color = basicColor();
  //vec4 color = BlinnPhong(gl_FragCoord.xyz, v_Normal);
  //color = vec4(v_normal,0.5);

  if (u_useDepthPeeling != 0)
  {
    peeling(color);
  }
  
  fragColor = color;
}