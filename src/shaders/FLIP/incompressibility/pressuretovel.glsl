#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout(std430, binding = 0) readonly buffer PressureBuffer { float[] pressure; };
layout(std430, binding = 1) readonly buffer IsAirBuffer { uint[] isAir; };
layout(std430, binding = 2) buffer VelXBuffer { float[] velX; };
layout(std430, binding = 3) buffer VelYBuffer { float[] velY; };
layout(std430, binding = 4) buffer VelZBuffer { float[] velZ; };

uniform int gridX;
uniform int gridY;
uniform int gridZ;
uniform float h;

#pragma include "../utils.glsl"

void main(){
    int i = int(gl_GlobalInvocationID.x);
    int j = int(gl_GlobalInvocationID.y);
    int k = int(gl_GlobalInvocationID.z);
    if (i >= gridX || j >= gridY || k >= gridZ) return;

    int cell = coordToCell(ivec3(i,j,k), gridX, gridY, gridZ);
    bool cellIsFluid = isAir[cell] == 0u;
    float p = cellIsFluid ? pressure[cell] : 0.0;

    if (i + 1 < gridX) {
        int rightCell = coordToCell(ivec3(i+1, j, k), gridX, gridY, gridZ);
        bool rightIsFluid = isAir[rightCell] == 0u;
        if (cellIsFluid || rightIsFluid) {
            int rightFace = coordToCell(ivec3(i+1, j, k), gridX+1, gridY, gridZ);
            float pRight = rightIsFluid ? pressure[rightCell] : 0.0;
            velX[rightFace] += (p - pRight);
        }
    }
    if (j + 1 < gridY) {
        int topCell = coordToCell(ivec3(i, j+1, k), gridX, gridY, gridZ);
        bool topIsFluid = isAir[topCell] == 0u;
        if (cellIsFluid || topIsFluid) {
            int topFace = coordToCell(ivec3(i, j+1, k), gridX, gridY+1, gridZ);
            float pTop = topIsFluid ? pressure[topCell] : 0.0;
            velY[topFace] += (p - pTop);
        }
    }
    if (k + 1 < gridZ) {
        int frontCell = coordToCell(ivec3(i, j, k+1), gridX, gridY, gridZ);
        bool frontIsFluid = isAir[frontCell] == 0u;
        if (cellIsFluid || frontIsFluid) {
            int frontFace = coordToCell(ivec3(i, j, k+1), gridX, gridY, gridZ+1);
            float pFront = frontIsFluid ? pressure[frontCell] : 0.0;
            velZ[frontFace] += (p - pFront);
        }
    }
}