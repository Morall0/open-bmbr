#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform vec3 diffuseColor;
uniform int useTexture;

// Light structures
struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3  position;
    float constant;
    float linear;
    float quadratic;
    vec3  ambient;
    vec3  diffuse;
    vec3  specular;
};

// Uniforms
uniform DirLight  dirLight;
uniform PointLight bombLight;
uniform bool       bombLightActive;
uniform PointLight fireLight;
uniform bool       fireLightActive;
uniform vec3       viewPos;
uniform float      shininess;

// Fire-specific uniforms
uniform int  isFireModel; // 1 = fire mesh (emissive), 0 = regular mesh
uniform vec3 fireCenter;  // world-space origin of this fire instance

// Light calculation functions
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir,
                  vec3 baseDiffuse, vec3 baseSpecular)
{
    vec3  lightDir   = normalize(-light.direction);
    float diff       = max(dot(normal, lightDir), 0.0);
    vec3  reflectDir = reflect(-lightDir, normal);
    float spec       = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    vec3 ambient  = light.ambient  * baseDiffuse;
    vec3 diffuse  = light.diffuse  * diff * baseDiffuse;
    vec3 specular = light.specular * spec * baseSpecular;
    return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir,
                    vec3 baseDiffuse, vec3 baseSpecular)
{
    vec3  lightDir    = normalize(light.position - fragPos);
    float diff        = max(dot(normal, lightDir), 0.0);
    vec3  reflectDir  = reflect(-lightDir, normal);
    float spec        = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
                                light.quadratic * (distance * distance));

    vec3 ambient  = light.ambient  * baseDiffuse;
    vec3 diffuse  = light.diffuse  * diff * baseDiffuse;
    vec3 specular = light.specular * spec * baseSpecular;
    return (ambient + diffuse + specular) * attenuation;
}

// Fire gradient: white/hot core -> yellow -> orange edge
vec3 FireGradient(float t)
{
    vec3 core  = vec3(1.0,  1.0,  0.90); // near-white
    vec3 mid   = vec3(1.0,  0.80, 0.0);  // yellow
    vec3 outer = vec3(1.0,  0.25, 0.0);  // orange-red

    if (t < 0.5)
        return mix(core, mid,   t * 2.0);
    else
        return mix(mid,  outer, (t - 0.5) * 2.0);
}

// Main
void main()
{
    // Resolve base color from texture or material
    vec3 baseColor;
    if (useTexture == 1) {
        vec4 texColor = texture(texture_diffuse1, TexCoords);
        if (texColor.a < 0.1)
            discard;
        baseColor = texColor.rgb;
    } else {
        baseColor = diffuseColor;
    }

    // Fire emissive self-glow
    if (isFireModel == 1)
    {
        // Radius covering the fire model in world space
        float radius = 0.42;
        float dist   = length(FragPos - fireCenter);
        float t      = clamp(dist / radius, 0.0, 1.0); // 0=core, 1=edge

        vec3 fireColor = FireGradient(t);

        // At the edges blend in the material color for the orange/yellow tint.
        vec3 tintedColor = mix(fireColor, baseColor * fireColor, t * t);

        float brightness = mix(30.0, 1.5, t * t);
        vec3 emissive = tintedColor * brightness;

        // Aggressive tone-map: small denominator saturates core to white
        emissive = emissive / (emissive + vec3(0.10));
        emissive = pow(emissive, vec3(1.0 / 2.2)); // gamma

        FragColor = vec4(emissive, 1.0);
        return;
    }

    // full lighting pipeline for regular mesh robot, etc
    vec3 baseDiffuse  = baseColor;
    vec3 baseSpecular = baseColor * 0.3;

    vec3 norm    = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Directional light
    vec3 result = CalcDirLight(dirLight, norm, viewDir, baseDiffuse, baseSpecular);

    // Bomb point light
    if (bombLightActive)
        result += CalcPointLight(bombLight, norm, FragPos, viewDir, baseDiffuse, baseSpecular);

    // Fire point light (when fire is active)
    if (fireLightActive)
        result += CalcPointLight(fireLight, norm, FragPos, viewDir, baseDiffuse, baseSpecular);

    FragColor = vec4(result, 1.0);
}
