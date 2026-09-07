#version 430 core
out vec4 FragColor;
in vec3 vClipPos;

uniform sampler2D depthTex;
uniform mat4 uInvView;
uniform mat4 uInvProj;
uniform vec2 viewport;

vec4 viewPos(vec2 uv) {
    uv = clamp(uv, 0, 1);
    float depth = texture(depthTex, uv).x;
    if (depth <= 0) return vec4(0,0,0,-10000);
    vec2 ndc = uv * 2.0 - 1.0;
    vec4 clipPos = vec4(ndc, 0.0, 1.0);
    vec4 viewDir = uInvProj * clipPos;
    viewDir /= viewDir.w;
    
    vec3 rayDir = normalize(viewDir.xyz);
    return vec4(rayDir * (depth / (-rayDir.z)), depth);
}

vec3 safeNormalize(vec3 x){
    float len = length(x);
    return len > 0 ? x / len : vec3(0);
}

void main()
{
    vec2 uv = 0.5 * (vClipPos.xy + 1);
    vec4 posCentre = viewPos(uv);

    vec2 texelSize = 1 / viewport;
    vec4 ddx = viewPos(uv + texelSize * vec2(1, 0)) - posCentre;
    vec4 ddx2 = posCentre - viewPos(uv + texelSize * vec2(-1, 0));
    if (abs(ddx2.z) < abs(ddx.z) || ddx.w > 0.19){
        ddx = ddx2;
    }

    vec4 ddy = viewPos(uv + texelSize * vec2(0, 1)) - posCentre;
    vec4 ddy2 = posCentre - viewPos(uv + texelSize * vec2(0, -1));
    if (abs(ddy2.z) < abs(ddy.z)){
        ddy = ddy2;
    }

    vec3 viewNormal = safeNormalize(cross(ddx.xyz, ddy.xyz));
    vec4 worldNormal = uInvView * vec4(viewNormal, 0);

    FragColor = vec4(worldNormal.xyz, 1);
}