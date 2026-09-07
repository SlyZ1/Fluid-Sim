#version 430 core
out vec4 FragColor;
in vec3 vClipPos;

uniform sampler2D cumulativeDepthTex;
uniform sampler2D depthTex;
uniform sampler2D normalTex;
uniform mat4 uInvView;
uniform mat4 uInvProj;
uniform vec3 lookDir;
uniform vec3 cameraPos;

vec3 getRayDir(vec2 uv) {
    vec2 ndc = uv * 2.0 - 1.0;
    vec4 clipPos = vec4(ndc, -1.0, 1.0);
    vec4 viewDir = uInvProj * clipPos;
    viewDir.w = 0.0;
    vec3 worldDir = (uInvView * viewDir).xyz;
    return normalize(worldDir);
}

vec4 uvToViewPos(vec2 uv) {
    uv = clamp(uv, 0, 1);
    float depth = texture(depthTex, uv).x;
    if (depth <= 0) return vec4(0,0,0,-10000);
    vec2 ndc = uv * 2.0 - 1.0;
    vec4 clipPos = vec4(ndc, 0.0, 1.0);
    vec4 viewDir = uInvProj * clipPos;
    //viewDir /= viewDir.w;
    
    vec3 rayDir = normalize(viewDir.xyz);
    return vec4(rayDir * (depth / (-rayDir.z)), depth);
}

vec3 getSkyColor(vec3 dir){
    vec3 skyTop = vec3(0.1, 0.35, 0.85);
    vec3 skyHorizon = vec3(0.65, 0.8, 0.95);
    vec3 groundColor = vec3(0.3, 0.3, 0.28);

    float t = clamp(dir.y, 0.0, 1.0);
    float horizonFade = pow(1.0 - t, 3.0);

    vec3 skyColor = mix(skyTop, skyHorizon, horizonFade);
    skyColor = mix(skyColor, groundColor, step(dir.y, -.5));

    float sunDot = max(dot(dir, normalize(vec3(1.5,0.6,-1))), 0.0);
    
    float sunDisc = pow(sunDot, 2000.0);      // disque net, exposant très élevé pour un petit disque serré
    float sunGlow = pow(sunDot, 8.0) * 0.1;    // halo large et doux autour

    vec3 sunColor = vec3(1.0, 0.9, 0.7);
    skyColor += sunColor * sunDisc * 5.0;
    skyColor += sunColor * sunGlow;
    return skyColor;
}

void main()
{
    vec2 uv = 0.5 * (vClipPos.xy + 1);
    vec3 dir = getRayDir(uv);

    float cumulativeDepth = texture(cumulativeDepthTex, uv).x / 100;
    vec3 backgroundColor = getSkyColor(dir);
    if (cumulativeDepth <= 0) {
        FragColor = vec4(backgroundColor, 1);
        return;
    }

    vec4 waterColor = vec4(0.03, 0.23, 0.38, 1.0);
    vec3 transmittance = exp(-cumulativeDepth * (1 - waterColor.xyz));
    vec3 absorptionColor = transmittance * backgroundColor;
    //FragColor = absorptionColor;

    vec3 normal = normalize(texture(normalTex, uv).xyz);
    //normal = vec3(0,1,0);
    vec3 viewDir = -lookDir;
    vec4 viewPos = uvToViewPos(uv);
    vec3 worldPos = (uInvView * vec4(viewPos.xyz, 1.0)).xyz;
    viewDir = normalize(cameraPos - worldPos);
    // FragColor = vec4(normal, 1.0);
    // return;

    float f0 = (1 - 1.33) / (1 + 1.33);
    f0 *= f0;
    float fresnel = f0 + (1 - f0) * pow(1 - dot(viewDir, normal), 5);
    vec3 shading = (1 - fresnel) * absorptionColor + fresnel * getSkyColor(reflect(-viewDir, normal));

    FragColor = vec4(shading, 1);
}