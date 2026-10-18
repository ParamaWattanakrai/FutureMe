#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec3 VertexColour;
in float SkyBlend;

out vec4 colour;

uniform int uMode; // 0 = sky quad, 1 = lit scene

// Blinn-Phong: ambient + diffuse + specular, one directional light.
uniform vec3 lightDirection; // points FROM the light TOWARDS the scene
uniform vec3 lightColour;
uniform float ambientStrength;
uniform float specularStrength;
uniform float shininess;
uniform vec3 viewPos;

uniform vec2 resolution;
uniform float time;

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

vec3 skyColourAt(vec2 fragCoord)
{
    vec2 uv = fragCoord / resolution;

    vec3 topColour     = vec3(0.282, 0.498, 0.675);
    vec3 horizonColour = vec3(0.737, 0.831, 0.894);

    float t = pow(clamp(uv.y, 0.0, 1.0), 0.85);
    vec3 sky = mix(horizonColour, topColour, t);

    vec2 cloudUv = vec2(uv.x * 2.2 + time * 0.01, uv.y * 7.0);
    float clouds = fbm(cloudUv);
    clouds = smoothstep(0.45, 0.85, clouds);
    float cloudFade = smoothstep(1.0, 0.35, uv.y) * smoothstep(0.0, 0.15, uv.y);
    clouds *= cloudFade;

    return mix(sky, vec3(0.97, 0.98, 0.99), clouds * 0.75);
}

void main()
{
    if (uMode == 0)
    {
        colour = vec4(skyColourAt(gl_FragCoord.xy), 1.0);
    }
    else
    {
        // Top faces: always flat white. Else sky texture if SkyBlend, else flat colour.
        vec3 baseColour;
        if (Normal.y > 0.5)
            baseColour = vec3(1.0, 1.0, 1.0);
        else if (SkyBlend > 0.5)
            baseColour = skyColourAt(gl_FragCoord.xy);
        else
            baseColour = VertexColour;

        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(-lightDirection);

        vec3 ambient = ambientStrength * lightColour;

        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColour;

        // Blinn-Phong half-vector specular.
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
        vec3 specular = specularStrength * spec * lightColour;

        vec3 result = (ambient + diffuse + specular) * baseColour;
        colour = vec4(result, 1.0);
    }
}
