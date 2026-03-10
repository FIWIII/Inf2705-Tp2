#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 texCoords;

#define MAX_SPOT_LIGHTS 12

out ATTRIBS_VS_OUT
{
    vec2 texCoords;
    vec3 normal;
    vec3 color;
} attribsOut;

out LIGHTS_VS_OUT
{
    vec3 obsPos;
    vec3 dirLightDir;
    
    vec3 spotLightsDir[MAX_SPOT_LIGHTS];
    vec3 spotLightsSpotDir[MAX_SPOT_LIGHTS];
} lightsOut;

uniform mat4 mvp;
uniform mat4 view;
uniform mat4 modelView;
uniform mat3 normalMatrix;

struct Material
{
    vec3 emission;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct DirectionalLight
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    vec3 direction;
};

struct SpotLight
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    vec3 position;
    vec3 direction;
    float exponent;
    float openingAngle;
};

uniform int nSpotLights;

layout (std140) uniform MaterialBlock
{
    Material mat;
};

layout (std140) uniform LightingBlock
{
    DirectionalLight dirLight;
    SpotLight spotLights[MAX_SPOT_LIGHTS];
};

void main()
{
    
    vec4 viewPos4 = modelView * vec4(position, 1.0);
    vec3 viewPos = viewPos4.xyz;

    attribsOut.texCoords = texCoords;
    attribsOut.color = color;

    vec3 n = normalMatrix * normal;
    if (length(n) < 0.0001)
        n = vec3(0.0, 1.0, 0.0);
    attribsOut.normal = n;

    lightsOut.obsPos = -viewPos;
    lightsOut.dirLightDir = mat3(view) * (-dirLight.direction);

    for (int i = 0; i < nSpotLights; i++)
    {
        vec3 lightPosView = (view * vec4(spotLights[i].position, 1.0)).xyz;
        lightsOut.spotLightsDir[i] = lightPosView - viewPos;
        lightsOut.spotLightsSpotDir[i] = mat3(view) * (-spotLights[i].direction);
    }

    gl_Position = mvp * vec4(position, 1.0);
    
}
