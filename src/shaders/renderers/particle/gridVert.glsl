#version 430 core
layout (location = 0) in vec2 vPos;
layout (location = 1) in vec4 rect;
layout (location = 2) in vec4 color;
out vec2 vClipPos;
out vec4 vColor;

uniform vec2 viewport;

void main()
{
    vClipPos = vPos;
    vec2 centerAbsolute = rect.xy + viewport * 0.5;
    vec2 snappedCenter = floor(centerAbsolute) + 0.5;
    vec2 pixelPosAbsolute = snappedCenter + vPos * rect.zw;
    vec2 worldPos = (pixelPosAbsolute - viewport * 0.5) / (viewport * 0.5);
    gl_Position = vec4(worldPos, 0.0, 1.0);
    vColor = color;
}