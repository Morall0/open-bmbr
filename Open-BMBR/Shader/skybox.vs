#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    TexCoords = aTexCoords;
    
    // Remove translation from the view matrix
    mat4 rotView = mat4(mat3(view));
    
    // Calculate final position
    vec4 pos = projection * rotView * model * vec4(aPos, 1.0);
    
    // Force z to w, so that depth division results in z=1.0 (max depth)
    gl_Position = pos.xyww;
}
