#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D image;
uniform vec4 color;
uniform bool useTexture;

void main() {
    if (useTexture) {
        vec4 texColor = texture(image, TexCoords) * color;
        // Discard fully transparent pixels
        if(texColor.a < 0.1)
            discard;
        FragColor = texColor;
    } else {
        FragColor = color;
    }
}
