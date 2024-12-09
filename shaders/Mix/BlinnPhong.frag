//Light
uniform vec3 u_lightDirection;
uniform vec3 u_lightAmbient;
uniform vec3 u_lightDiffuse;
uniform vec3 u_lightSpecular;
uniform float u_lightIntensity;
uniform vec3 u_viewPosition;

//Material
//uniform vec4 u_materialAmbient; v_Color is used instead
uniform vec3 u_materialDiffuse;
uniform vec3 u_materialSpecular;
uniform float u_materialShininess;

/*
According to the Blinn-Phong reflection model : https://en.wikipedia.org/wiki/Blinn%E2%80%93Phong_reflection_model
*/
vec4 BlinnPhong(vec4 ambiant, vec3 fragPos, vec3 normal) {
    vec3 N = normalize(normal);
    vec3 L = normalize(-u_lightDirection);
    vec3 V = normalize(u_viewPosition - fragPos);
    vec3 H = normalize(L + V);

    // ambiant
    vec3 ambient = u_lightAmbient * ambiant.rgb;

    // diffuse
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = u_lightDiffuse * (diff * u_materialDiffuse);

    // specular
    float spec = pow(max(dot(N, H), 0.0), u_materialShininess);
    vec3 specular = u_lightSpecular * (spec * u_materialSpecular);

    // final color
    vec3 finalColor = (ambient + diffuse + specular) * u_lightIntensity;

    return vec4(finalColor, ambiant.a);
}