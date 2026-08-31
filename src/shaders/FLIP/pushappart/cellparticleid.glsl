#version 430 core
layout(local_size_x = 64) in;

layout(std430, binding = 0) buffer FirstCellParticleBuffer { uint firstCellParticle[]; };
layout(std430, binding = 1) readonly buffer CellOfBuffer { uint cellOf[]; };
layout(std430, binding = 2) writeonly buffer CellParticleIdsBuffer { uint cellParticleIds[]; };

uniform int partN;

void main(){
    int i = int(gl_GlobalInvocationID.x);
    if (i >= partN) return;

    uint cell = cellOf[i];
    uint slot = atomicAdd(firstCellParticle[cell], 1u);
    cellParticleIds[slot] = uint(i);
}