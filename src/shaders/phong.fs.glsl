#version 330 core

#define MAX_SPOT_LIGHTS 12

in ATTRIBS_VS_OUT
{
    vec2 texCoords;
    vec3 normal;
    vec3 color;
} attribsIn;

in LIGHTS_VS_OUT
{
    vec3 obsPos;
    vec3 dirLightDir;
    
    vec3 spotLightsDir[MAX_SPOT_LIGHTS];
    vec3 spotLightsSpotDir[MAX_SPOT_LIGHTS];
} lightsIn;


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

uniform vec3 globalAmbient;

layout (std140) uniform MaterialBlock
{
    Material mat;
};

layout (std140) uniform LightingBlock
{
    DirectionalLight dirLight;
    SpotLight spotLights[MAX_SPOT_LIGHTS];
};

uniform sampler2D diffuseSampler;

out vec4 FragColor;


float computeSpot(in float openingAngle, in float exponent, in vec3 spotDir, in vec3 lightDir, in vec3 normal)
{
    
    vec3 L = normalize(lightDir);
    vec3 S = normalize(spotDir);

    float cosGamma = dot(L, S);
    float cosDelta = cos(radians(openingAngle));

    if (cosGamma > cosDelta)
        return pow(cosGamma, exponent);

    return 0.0;
}

void main()
{
    vec3 N = normalize(attribsIn.normal);
    vec3 O = normalize(lightsIn.obsPos);

    vec4 texColor = texture(diffuseSampler, attribsIn.texCoords);
    vec3 baseColor = texColor.rgb;

    vec3 color = mat.emission;
    color += globalAmbient * mat.ambient * baseColor;

    vec3 Ld = normalize(lightsIn.dirLightDir);
    float diff = max(dot(N, Ld), 0.0);

    const float LEVELS = 4.0;
    diff = floor(diff * LEVELS) / LEVELS;

    vec3 R = reflect(-Ld, N);
    float spec = pow(max(dot(R, O), 0.0), mat.shininess);
    spec = floor(spec * LEVELS) / LEVELS;

    color += dirLight.ambient * mat.ambient * baseColor;
    color += dirLight.diffuse * diff * mat.diffuse * baseColor;
    color += dirLight.specular * spec * mat.specular;

    
    for(int i = 0; i < nSpotLights; i++)
    {
        
        vec3 L = normalize(lightsIn.spotLightsDir[i]);
        float d = length(lightsIn.spotLightsDir[i]);

        float spot = computeSpot(
            spotLights[i].openingAngle,
            spotLights[i].exponent,
            lightsIn.spotLightsSpotDir[i],
            lightsIn.spotLightsDir[i],
            N
        );

        float attenuation = 1.0 - smoothstep(7.0, 10.0, d);

        float sdiff = max(dot(N, L), 0.0);
        vec3 SR = reflect(-L, N);
        float sspec = pow(max(dot(SR, O), 0.0), mat.shininess);

        color += spotLights[i].ambient * mat.ambient * baseColor * spot;
        color += spotLights[i].diffuse * sdiff * mat.diffuse * baseColor * spot * attenuation;
        color += spotLights[i].specular * sspec * mat.specular * spot * attenuation;
    }

    FragColor = vec4(color, texColor.a);
}
