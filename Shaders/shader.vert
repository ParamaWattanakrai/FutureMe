#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec4 aColourBlend; // .rgb colour, .a sky-blend

out vec3 FragPos;
out vec3 Normal;
out vec3 VertexColour;
out float SkyBlend;

uniform int uMode; // 0 = sky quad, 1 = lit scene
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;

void main()
{
    if (uMode == 0)
    {
        FragPos = vec3(0.0);
        Normal = vec3(0.0, 1.0, 0.0);
        VertexColour = vec3(0.0);
        SkyBlend = 0.0;
        gl_Position = vec4(aPos.xy, 0.999, 1.0);
    }
    else
    {
        vec4 worldPos = model * vec4(aPos, 1.0);
        FragPos = worldPos.xyz;
        Normal = normalize(normalMatrix * aNormal);
        VertexColour = aColourBlend.rgb;
        SkyBlend = aColourBlend.a;
        gl_Position = projection * view * worldPos;
    }
}
