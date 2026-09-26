#version 430 core

layout (location = 0) in float lineT;
layout (location = 1) in vec3 start;
layout (location = 2) in vec3 end;
layout (location = 3) in vec4 color;
out vec4 lineColor;

uniform mat4 uProj;
uniform mat4 uView;

void main()
{
    lineColor = color;
    vec4 worldPos = vec4(mix(start, end, lineT), 1);
    gl_Position = uProj * uView * worldPos;
}