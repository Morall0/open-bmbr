#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform vec3 diffuseColor;
uniform int useTexture;

struct PointLight {
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform PointLight bombLight;
uniform bool bombLightActive;
uniform PointLight fireLight;
uniform bool fireLightActive;
uniform vec3 viewPos;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    vec3 ambient  = light.ambient  * baseColor;
    vec3 diffuse  = light.diffuse  * diff * baseColor;
    vec3 specular = light.specular * spec * baseColor;
    return (ambient + diffuse + specular) * attenuation;
}

void main()
{
    vec3 baseColor;
    if (useTexture == 1) {
        vec4 texColor = texture(texture_diffuse1, TexCoords);
        if (texColor.a < 0.1)
            discard;
        baseColor = texColor.rgb;
    } else {
        baseColor = diffuseColor;
    }

    vec3 result = baseColor;

    if (bombLightActive)
    {
        vec3 norm = normalize(Normal);
        vec3 viewDir = normalize(viewPos - FragPos);
        result += CalcPointLight(bombLight, norm, FragPos, viewDir, baseColor);
    }

    if (fireLightActive)
    {
        vec3 norm = normalize(Normal);
        vec3 viewDir = normalize(viewPos - FragPos);
        result += CalcPointLight(fireLight, norm, FragPos, viewDir, baseColor);
    }

    FragColor = vec4(result, 1.0);
}