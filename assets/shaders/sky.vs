#version 330

// Dome de ciel : sphere unitaire centree sur la camera. La position modele
// du sommet sert directement de direction de visee au fragment.

in vec3 vertexPosition;

uniform mat4 mvp;

out vec3 fragDir;

void main()
{
    fragDir = vertexPosition;
    gl_Position = mvp*vec4(vertexPosition, 1.0);
}
