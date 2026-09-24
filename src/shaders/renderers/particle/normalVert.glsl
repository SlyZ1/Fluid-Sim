#version 430 core
layout (location = 0) in vec3 vPos;
out vec3 vClipPos;

void main()
{
    vClipPos = vPos;
    gl_Position = vec4(vPos, 1.0);
}