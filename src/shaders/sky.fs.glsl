#version 330 core

in vec3 texCoords;

uniform samplerCube textureSampler;

out vec4 FragColor;

void main()
{
    // TODO: Skybox
    FragColor = texture(textureSampler, texCoords);
}
