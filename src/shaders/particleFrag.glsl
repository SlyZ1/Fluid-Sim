#version 430 core
out vec4 FragColor;
in vec3 vClipPos;
in vec4 vColor;

void main()
{
    FragColor = vec4(0.02, 0.51, 1.0, 0.1);
}