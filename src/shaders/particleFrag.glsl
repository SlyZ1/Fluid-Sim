#version 430 core
out vec4 FragColor;
in vec3 vClipPos;
in vec4 vColor;

void main()
{
    float dist = length(vClipPos.xy);
    if (dist > 1.0)
        discard;
    float aa = fwidth(dist);
    float alpha = 1.0 - smoothstep(1.0 - aa, 1.0 + aa, dist);
    
    FragColor = vec4(0.07, 0.55, 0.87, 0.1);
    FragColor = vec4(vColor.xyz, 0.9 * alpha);
}