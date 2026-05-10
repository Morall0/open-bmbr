#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

void main()
{    
    
  vec4   texColor= texture(texture_diffuse1, TexCoords);
    if(texColor.a < 0.1)
       discard;
    FragColor = texColor;
    // Quita el discard para ver todas las partes
    //FragColor = texColor.a < 0.1 ? vec4(1.0, 0.5, 0.0, 1.0) : texColor;
     //FragColor = vec4(texColor.rgb, 1.0);
}





