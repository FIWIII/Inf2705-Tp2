#version 330 core

layout (location = 0) in vec3 position;
layout (location = 2) in vec3 normal;

uniform mat4 mvp;

void main()
{
    // TODO: Modifier la position des sommets le long des normales. On utilise un facteur de 0.05.
    vec3 displacedPosition = position + normal * 0.05;
    gl_Position = mvp * vec4(displacedPosition, 1.0);
}
