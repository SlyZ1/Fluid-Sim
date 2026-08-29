#version 430 core
layout(local_size_x = 64) in;

layout(std430, binding = 0) buffer NumCellParticlesBuffer { uint[] numCellParticles; };
layout(std430, binding = 1) writeonly buffer CellOfBuffer { uint[] cellOf; };
layout(std430, binding = 2) readonly buffer PartPosBuffer { vec2[] partPos; };

uniform int partN;
uniform int gridX;
uniform int gridY;
uniform float h;

#pragma include "../utils.glsl"

void main(){
    int i = int(gl_GlobalInvocationID.x);
    if (i >= partN) return;

    int cell = posToCell(partPos[i], gridX, gridY);
    cellOf[i] = uint(cell);
    atomicAdd(numCellParticles[i], 1u);
}