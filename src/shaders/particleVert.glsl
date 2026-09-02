#version 430 core
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 cPos;
layout (location = 2) in vec4 color;
out vec3 vClipPos;
out vec4 vColor;

uniform float particleRadius;
uniform mat4 uView;
uniform mat4 uProj;

void main()
{
    vClipPos = vPos;
    vec4 worldPos = vec4((cPos + vPos * particleRadius), 1.0);
    worldPos.z -= 1000;
    
    gl_Position = uProj * uView * worldPos;
    vColor = color;
}