#version 430 core
layout (location = 0) in vec2 vPos;
layout (location = 1) in vec2 cPos;
layout (location = 2) in vec4 color;
out vec2 vClipPos;
out vec4 vColor;

uniform vec2 viewport;
uniform float particleRadius;
uniform mat4 uView;

void main()
{
    vClipPos = vPos;
    vec4 worldPos = vec4((cPos + vPos * particleRadius) * vec2(1, viewport.x / viewport.y), 0.0, 1.0);
    vec4 viewPos = uView * worldPos;

    const float minDepth = 0.1;
    const float maxDepth = 1000;
    float depth = (-viewPos.z - minDepth) / (maxDepth - minDepth);
    vec2 screenPos = viewPos.xy / max(-viewPos.z, minDepth);
    gl_Position = vec4(screenPos, 0.0, 1.0);
    vColor = color;
}