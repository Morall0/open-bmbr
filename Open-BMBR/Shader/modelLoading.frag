#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform vec3 diffuseColor;
uniform int useTexture;

void main()
{    
    if (useTexture == 1) {
        vec4 texColor = texture(texture_diffuse1, TexCoords);
        if(texColor.a < 0.1)
            discard;
        FragColor = texColor;
    } else {
        FragColor = vec4(diffuseColor, 1.0);
    }
}