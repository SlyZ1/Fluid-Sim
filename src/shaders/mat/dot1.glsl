#version 430 core
layout(local_size_x = 128) in;

layout(std430, binding = 0) readonly buffer InputU { float U[]; };
layout(std430, binding = 1) readonly buffer InputV { float V[]; };
layout(std430, binding = 2) writeonly buffer PartialResults { float partials[]; }; // taille = numWorkGroups

shared float partialSum[128];

uniform int N;

void main(){
    uint n = gl_GlobalInvocationID.x;
    uint i = gl_LocalInvocationID.x;
    partialSum[i] = (n < N) ? U[n] * V[n] : 0.0;

    barrier();
    for(uint stride = gl_WorkGroupSize.x / 2; stride > 0; stride >>= 1) {
        if (i < stride)
            partialSum[i] += partialSum[i + stride];
        barrier();
    }

    if (i == 0)
        partials[gl_WorkGroupID.x] = partialSum[0];
}