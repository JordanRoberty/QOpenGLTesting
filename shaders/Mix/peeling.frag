 uniform sampler2D u_previousDepthTexture;

 uniform int u_layer;

// we can use the previous depth texture because the texture size is the same as the color texture
 vec2 getTexCoord()
 {
    return gl_FragCoord.xy / textureSize(u_previousDepthTexture, 0);
 }

 void peeling(vec4 color)
 {
    if(u_layer != 0)// perform the peeling
    {
      vec2 texCoord = getTexCoord();
      vec4 previousDepth = texture(u_previousDepthTexture, texCoord);
      float epsilon = 0.0000001;
      if(gl_FragCoord.z <= previousDepth.r + epsilon)
      {
        discard;
      }
    }
 }