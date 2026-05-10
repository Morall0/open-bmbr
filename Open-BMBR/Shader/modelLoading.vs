//#version 330 core
//layout (location = 0) in vec3 aPos;
//layout (location = 1) in vec3 aNormal;
//layout (location = 2) in vec2 aTexCoords;

//out vec2 TexCoords;

//uniform mat4 model;
//uniform mat4 view;
//uniform mat4 projection;

//void main()
//{
  //  TexCoords = aTexCoords;    
    //gl_Position = projection * view * model * vec4(aPos, 1.0);
//}

#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;    // Tangent
layout (location = 4) in vec3 aBitangent;  // Bitangent
layout (location = 5) in ivec4 aBoneIDs;   // ✅ location 5
layout (location = 6) in vec4 aWeights;    // ✅ location 6

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

void main()
{
    vec4 totalPosition = vec4(0.0);

    for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        if (aBoneIDs[i] == -1) continue;
        if (aBoneIDs[i] >= MAX_BONES)
        {
            totalPosition = vec4(aPos, 1.0);
            break;
        }
        totalPosition += finalBonesMatrices[aBoneIDs[i]] * vec4(aPos, 1.0) * aWeights[i];
    }

    TexCoords = aTexCoords;
    gl_Position = projection * view * model * totalPosition;
}