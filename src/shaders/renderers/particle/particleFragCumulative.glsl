#version 430 core
out vec4 FragColor;
in vec3 vClipPos;
in vec4 vColor;
in float depth;
in float radius;

void main()
{
    float dist = length(vClipPos.xy);
    if (dist > 1.0)
        discard;
        
    float thicknessContribution = 2.0 * radius * sqrt(max(0.0, 1.0 - dot(vClipPos.xy, vClipPos.xy)));
    FragColor = vec4(vec3(thicknessContribution), 1);
}