#version 330 core

// Full-screen quad, already in NDC.
layout (location = 0) in vec3 aPos;

out vec2 uv;

void main()
{
    uv = aPos.xy * 0.5 + 0.5; // 0..1 across the screen
    gl_Position = vec4(aPos.xy, 0.999, 1.0); // pinned to the far plane
}
