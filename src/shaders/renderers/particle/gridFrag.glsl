#version 430 core
out vec4 FragColor;

in vec4 vColor;

uniform vec2 viewport;

void main()
{
    FragColor = vColor;
}