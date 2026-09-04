#version 430 core
layout(local_size_x = 256) in;

layout(std430, binding = 0) buffer PartPosBuffer { vec4[] partPos; };
layout(std430, binding = 1) buffer PartVelBuffer { vec4[] partVel; };

uniform float dt;
uniform int partN;

void main(){
    uint i = gl_GlobalInvocationID.x;
    if (i >= partN) return;

    vec3 acc = vec3(0,-9.81,0);
    vec3 newVel = partVel[i].xyz + acc * dt;
    partVel[i].xyz = newVel;
    partPos[i].xyz += newVel * dt;
}