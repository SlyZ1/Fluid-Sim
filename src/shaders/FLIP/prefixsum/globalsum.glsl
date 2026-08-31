#version 430 core
layout(local_size_x = 256) in;

layout(std430, binding = 0) buffer ResultBuffer { uint[] result; };
layout(std430, binding = 1) readonly buffer BlockSumScannedBuffer { uint[] blockSumScanned; };

uniform int length;

void main(){
    uint i = gl_GlobalInvocationID.x;
    uint blockIdx = gl_WorkGroupID.x;
    if (2*i >= length) return;

    result[2*i] += blockSumScanned[blockIdx];
    result[2*i+1] += blockSumScanned[blockIdx];
}