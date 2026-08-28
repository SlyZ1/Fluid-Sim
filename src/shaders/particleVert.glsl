#version 430 core
layout (location = 0) in vec2 vPos;
layout (location = 1) in vec2 cPos;
layout (location = 2) in vec4 color;
out vec2 vClipPos;
out vec4 vColor;

uniform vec2 viewport;
uniform float particleRadius;

void main()
{
    vClipPos = vPos;
    vec2 worldPos = (cPos + vPos * particleRadius) / (viewport*0.5);
    gl_Position = vec4(worldPos, 0.0, 1.0);
    vColor = color;
}