#version 430 core
out vec4 FragColor;
in vec3 vClipPos;

uniform sampler2D depthColorTex;
uniform sampler2D normalTex;
uniform vec2 viewport;
uniform mat4 uInvProj;

float gaussianKernel(vec2 x, float sigma){
    return exp(-dot(x, x) * 0.5 / (sigma * sigma));
}

float calculateScreenSpaceRadius(vec3 viewPoint, float worldRadius, float imageWidth) {
    float clipW = -viewPoint.z;
    float m00 = inverse(uInvProj)[0][0];    
    float pxPerMeter = (imageWidth * m00) / (2.0 * clipW);
    return abs(pxPerMeter * worldRadius);
}

vec4 uvToViewPos(vec2 uv) {
    uv = clamp(uv, 0, 1);
    float depth = texture(depthColorTex, uv).x;
    if (depth <= 0) return vec4(0,0,0,-10000);
    vec2 ndc = uv * 2.0 - 1.0;
    vec4 clipPos = vec4(ndc, 0.0, 1.0);
    vec4 viewDir = uInvProj * clipPos;
    //viewDir /= viewDir.w;
    
    vec3 rayDir = normalize(viewDir.xyz);
    return vec4(rayDir * (depth / (-rayDir.z)), depth);
}

void main()
{
    vec2 uv = 0.5 * (vClipPos.xy + 1);
    vec2 texelSize = 1 / viewport;
    float currentDepth = texture(depthColorTex, uv).x;
    if (currentDepth <= 0) discard;
    vec3 blurredVal = vec3(0);
    float totalWeight = 0;
    float radius = 10;

    vec4 viewPos = uvToViewPos(uv);
    float worldSpaceBlurRadius = 10;
    float screenRadius = calculateScreenSpaceRadius(viewPos.xyz, worldSpaceBlurRadius, 800);
    screenRadius = clamp(screenRadius, 1.0, 20.0);
    float sigmaRange = screenRadius;

    for(int x = -int(screenRadius); x <= int(screenRadius); x++) {
        for(int y = -int(screenRadius); y <= int(screenRadius); y++) {
            vec2 pos = vec2(x, y);
            vec2 offset = pos * texelSize;
            float depth = texture(depthColorTex, uv + offset).x;
            vec3 val = texture(normalTex, uv + offset).xyz;
            float rangeDiff = depth - currentDepth;
            
            float rangeWeight = exp(-(rangeDiff*rangeDiff) / (2.0 * sigmaRange * sigmaRange));
            float weight = gaussianKernel(pos, radius / 2.0) * rangeWeight;
            blurredVal += weight * val;
            totalWeight += weight;
        }
    }

    if (totalWeight > 0) 
        FragColor = vec4(blurredVal / totalWeight, 1);
    else
        FragColor = vec4(0,0,0,1);
}