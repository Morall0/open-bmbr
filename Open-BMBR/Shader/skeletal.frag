#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform vec3 diffuseColor;
uniform int useTexture;

// Fire shading mode
uniform int  isFireModel; // 1 = fire mesh, 0 = regular mesh
uniform vec3 fireCenter;  // oigin of the fire instance

struct PointLight {
    vec3  position;
    float constant;
    float linear;
    float quadratic;
    vec3  ambient;
    vec3  diffuse;
    vec3  specular;
};

uniform PointLight fireLight;
uniform bool       fireLightActive;
uniform vec3       viewPos;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor)
{
    vec3  lightDir    = normalize(light.position - fragPos);
    float diff        = max(dot(normal, lightDir), 0.0);
    vec3  reflectDir  = reflect(-lightDir, normal);
    float spec        = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
                                light.quadratic * (distance * distance));
    vec3 ambient  = light.ambient  * baseColor;
    vec3 diffuse  = light.diffuse  * diff * baseColor;
    vec3 specular = light.specular * spec * baseColor;
    return (ambient + diffuse + specular) * attenuation;
}

// Fire color white/hot core -> yellow -> orange edge
vec3 FireGradient(float t)
{
    vec3 core  = vec3(1.0,  1.0,  0.90); // bright white-yellow
    vec3 mid   = vec3(1.0,  0.80, 0.0);  // yellow
    vec3 outer = vec3(1.0,  0.25, 0.0);  // orange-red

    if (t < 0.5)
        return mix(core, mid,   t * 2.0);
    else
        return mix(mid,  outer, (t - 0.5) * 2.0);
}

void main()
{
    // Base color from texture or material
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

        // Brightness 30x at core, 1.5x at edge
        float brightness = mix(30.0, 1.5, t * t);
        vec3 emissive = tintedColor * brightness;

        // Aggressive tone-map: small denominator → core saturates to pure white
        emissive = emissive / (emissive + vec3(0.10));
        emissive = pow(emissive, vec3(1.0 / 2.2)); // gamma

        FragColor = vec4(emissive, 1.0);
        return;
    }

    // Regular mesh receive fire point light
    vec3 result = baseColor;

    if (fireLightActive)
    {
        vec3 norm    = normalize(Normal);
        vec3 viewDir = normalize(viewPos - FragPos);
        result += CalcPointLight(fireLight, norm, FragPos, viewDir, baseColor);
    }

    FragColor = vec4(result, 1.0);
}
