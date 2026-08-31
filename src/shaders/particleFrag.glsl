#version 430 core
out vec4 FragColor;
in vec2 vClipPos;
in vec4 vColor;

void main()
{
    float dist = length(vClipPos);
    float aa = fwidth(dist);
    float alpha = 1.0 - smoothstep(1.0 - aa, 1.0 + aa, dist);
    FragColor = vec4(vColor.rgb, 1);
}