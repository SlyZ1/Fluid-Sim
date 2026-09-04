#version 430 core
layout(local_size_x = 1024) in;

layout(std430, binding = 0) readonly buffer PartialResults { float partials[]; };
layout(std430, binding = 1) writeonly buffer Output { float result[]; };

shared float partialSum[1024];

uniform int numPartials;
uniform ivec3 indicies;

void main(){
    uint i = gl_LocalInvocationID.x;
    partialSum[i] = (i < numPartials) ? partials[i] : 0.0;

    barrier();
    for(uint stride = gl_WorkGroupSize.x / 2; stride > 0; stride >>= 1) {
        if (i < stride)
            partialSum[i] += partialSum[i + stride];
        barrier();
    }

    if (i == 0)
        for(int k = 0; k < 3; k++) {
            if (indicies[k] >= 0) result[indicies[k]] = partialSum[0];
        }
}