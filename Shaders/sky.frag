#version 330 core

in vec2 uv;
out vec4 colour;

uniform float time; // for cloud drift

// Value noise, for the clouds.
float hash(vec2 p)
{
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

float noise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);

    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

float fbm(vec2 p)
{
    float value = 0.0;
    float amplitude = 0.5;
    for (int i = 0; i < 5; ++i)
    {
        value += amplitude * noise(p);
        p *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

void main()
{
    // Blue at the top, pale near the horizon.
    vec3 topColour    = vec3(0.282, 0.498, 0.675);
    vec3 horizonColour = vec3(0.737, 0.831, 0.894);

    float t = pow(uv.y, 0.85);
    vec3 sky = mix(horizonColour, topColour, t);

    // Wispy horizontal clouds, fading out near the top.
    vec2 cloudUv = vec2(uv.x * 2.2 + time * 0.01, uv.y * 7.0);
    float clouds = fbm(cloudUv);
    clouds = smoothstep(0.45, 0.85, clouds);
    float cloudFade = smoothstep(1.0, 0.35, uv.y) * smoothstep(0.0, 0.15, uv.y);
    clouds *= cloudFade;

    vec3 finalColour = mix(sky, vec3(0.97, 0.98, 0.99), clouds * 0.75);

    colour = vec4(finalColour, 1.0);
}
