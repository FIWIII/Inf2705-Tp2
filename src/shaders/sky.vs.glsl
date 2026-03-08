#version 330 core

layout (location = 0) in vec3 position;

out vec3 texCoords;

uniform mat4 mvp;

void main()
{
    // TODO: Skybox
    texCoords = position;
    gl_Position = mvp * vec4(position, 1.0);
}
