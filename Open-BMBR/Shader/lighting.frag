#version 330 core

// #define NUMBER_OF_POINT_LIGHTS 0

struct Material
{
    sampler2D diffuse;         // Kd texture
    float ambientStrength;     // Ka scalar
    float specularStrength;    // Ks scalar
    float shininess;
};

struct DirLight
{
    vec3 direction;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight
{
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight
{
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    
    float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 color;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight bombLight;
uniform bool bombLightActive;
uniform PointLight fireLight;
uniform bool fireLightActive;
// uniform SpotLight spotLight;
uniform Material material;
uniform int transparency;
uniform vec2 uvScale;

// Function prototypes
vec3 CalcDirLight( DirLight light, vec3 normal, vec3 viewDir );
vec3 CalcPointLight( PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir );
vec3 CalcSpotLight( SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir );

void main( )
{
    // Properties
    vec3 norm = normalize( Normal );
    vec3 viewDir = normalize( viewPos - FragPos );
    
    // Directional lighting
    vec3 result = CalcDirLight( dirLight, norm, viewDir );
    
    // Bomb point light
    if (bombLightActive)
    {
        result += CalcPointLight( bombLight, norm, FragPos, viewDir );
    }

    // Fire point light
    if (fireLightActive)
    {
        result += CalcPointLight( fireLight, norm, FragPos, viewDir );
    }
    
    // Spot light
    // result += CalcSpotLight( spotLight, norm, FragPos, viewDir );
 	
    color = vec4(result, 1.0);
	  if(color.a < 0.1 && transparency==1)
        discard;

}

// Calculates the color when using a directional light.
vec3 CalcDirLight( DirLight light, vec3 normal, vec3 viewDir )
{
    vec3 lightDir = normalize( -light.direction );
    
    // Diffuse shading
    float diff = max( dot( normal, lightDir ), 0.0 );
    
    // Specular shading
    vec3 reflectDir = reflect( -lightDir, normal );
    float spec = pow( max( dot( viewDir, reflectDir ), 0.0 ), material.shininess );

    // Sample diffuse texture
    vec3 texColor = vec3(texture(material.diffuse, TexCoords * uvScale));
    
    // Phong components: Ka, Kd, Ks
    vec3 ambient  = light.ambient  * material.ambientStrength  * texColor;
    vec3 diffuse  = light.diffuse  * diff                      * texColor;
    vec3 specular = light.specular * material.specularStrength * spec * texColor;
    
    return ( ambient + diffuse + specular );
}

// Calculates the color when using a point light.
vec3 CalcPointLight( PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir )
{
    vec3 lightDir = normalize( light.position - fragPos );
    
    // Diffuse shading
    float diff = max( dot( normal, lightDir ), 0.0 );
    
    // Specular shading
    vec3 reflectDir = reflect( -lightDir, normal );
    float spec = pow( max( dot( viewDir, reflectDir ), 0.0 ), material.shininess );
    
    // Attenuation
    float distance = length( light.position - fragPos );
    float attenuation = 1.0f / ( light.constant + light.linear * distance + light.quadratic * ( distance * distance ) );

    // Sample diffuse texture
    vec3 texColor = vec3(texture(material.diffuse, TexCoords));

    // Phong components: Ka, Kd, Ks
    vec3 ambient  = light.ambient  * material.ambientStrength  * texColor;
    vec3 diffuse  = light.diffuse  * diff                      * texColor;
    vec3 specular = light.specular * material.specularStrength * spec * texColor;
    
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    
    return ( ambient + diffuse + specular );
}

// Calculates the color when using a spot light.
vec3 CalcSpotLight( SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir )
{
    vec3 lightDir = normalize( light.position - fragPos );
    
    // Diffuse shading
    float diff = max( dot( normal, lightDir ), 0.0 );
    
    // Specular shading
    vec3 reflectDir = reflect( -lightDir, normal );
    float spec = pow( max( dot( viewDir, reflectDir ), 0.0 ), material.shininess );
    
    // Attenuation
    float distance = length( light.position - fragPos );
    float attenuation = 1.0f / ( light.constant + light.linear * distance + light.quadratic * ( distance * distance ) );
    
    // Spotlight intensity
    float theta = dot( lightDir, normalize( -light.direction ) );
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp( ( theta - light.outerCutOff ) / epsilon, 0.0, 1.0 );
    
    // Combine results
    vec3 texColor = vec3( texture( material.diffuse, TexCoords ) );

    // Phong components: Ka, Kd, Ks
    vec3 ambient  = light.ambient  * material.ambientStrength  * texColor;
    vec3 diffuse  = light.diffuse  * diff                      * texColor;
    vec3 specular = light.specular * material.specularStrength * spec * texColor;
    
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    
    return ( ambient + diffuse + specular );
}
