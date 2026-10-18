#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec4 aColourBlend; // .rgb colour, .a sky-blend

out vec3 FragPos;
out vec3 Normal;
out vec3 VertexColour;
out float SkyBlend;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix; // inverse-transpose(model), from the CPU

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;
    Normal = normalize(normalMatrix * aNormal);
    VertexColour = aColourBlend.rgb;
    SkyBlend = aColourBlend.a;

    gl_Position = projection * view * worldPos;
}
