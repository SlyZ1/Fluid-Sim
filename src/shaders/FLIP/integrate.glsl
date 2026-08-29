#version 430 core
layout(local_size_x = 256) in;

layout(std430, binding = 0) buffer PartPosBuffer { vec2[] partPos; };
layout(std430, binding = 1) buffer PartVelBuffer { vec2[] partVel; };

uniform float dt;
uniform int partN;

void main(){
    uint i = gl_GlobalInvocationID.x;
    if (i >= partN) return;

    vec2 newVel = partVel[i] + vec2(0, -9.81) * dt;
    partVel[i] = newVel;
    partPos[i] += newVel * dt;
}