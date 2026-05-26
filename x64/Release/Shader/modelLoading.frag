#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform vec3 diffuseColor;
uniform int useTexture;


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
    float spec        = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
                                light.quadratic * (distance * distance));

    vec3 ambient  = light.ambient  * baseDiffuse;
    vec3 diffuse  = light.diffuse  * diff * baseDiffuse;
    vec3 specular = light.specular * spec * baseSpecular;
    return (ambient + diffuse + specular) * attenuation;
}

// Main
void main()
{
    // Resolve base colors from texture or material
    vec3 baseDiffuse;
    if (useTexture == 1) {
        vec4 texColor = texture(texture_diffuse1, TexCoords);
        if (texColor.a < 0.1)
            discard;
        baseDiffuse = texColor.rgb;
    } else {
        baseDiffuse = diffuseColor;
    }
    vec3 baseSpecular = baseDiffuse * 0.3; // low specular for untextured objects

    vec3 norm    = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Directional light (sun / ambient scene light)
    vec3 result = CalcDirLight(dirLight, norm, viewDir, baseDiffuse, baseSpecular);

    // Bomb point light
    if (bombLightActive)
        result += CalcPointLight(bombLight, norm, FragPos, viewDir, baseDiffuse, baseSpecular);

    // Fire point light
    if (fireLightActive)
        result += CalcPointLight(fireLight, norm, FragPos, viewDir, baseDiffuse, baseSpecular);

    FragColor = vec4(result, 1.0);
}
