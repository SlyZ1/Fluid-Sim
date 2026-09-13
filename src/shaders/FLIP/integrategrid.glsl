#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout(std430, binding = 1) readonly buffer CurvatureBuffer { float[] curvature; };
layout(std430, binding = 2) readonly buffer SmoothRhoBuffer { float[] smoothRho; };
layout(std430, binding = 3) buffer VelXBuffer { float[] velX; };
layout(std430, binding = 4) buffer VelYBuffer { float[] velY; };
layout(std430, binding = 5) buffer VelZBuffer { float[] velZ; };

uniform int gridX;
uniform int gridY;
uniform int gridZ;
uniform float h;
uniform float dt;
uniform float sigma;

const float rho0 = 8.0;

#pragma include "./utils.glsl"

// Balanced-force CSF (Francois et al. 2006):
float tensionDv(int cell, int neighbourCell){
    float kappaFace = 0.5 * (curvature[cell] + curvature[neighbourCell]);
    float dc = (smoothRho[cell] - smoothRho[neighbourCell]) / rho0;
    return 2.0 * sigma * kappaFace * dc * dt;
}

void main(){
    int i = int(gl_GlobalInvocationID.x);
    int j = int(gl_GlobalInvocationID.y);
    int k = int(gl_GlobalInvocationID.z);
    if (i >= gridX || j >= gridY || k >= gridZ) return;
    int cell = coordToCell(ivec3(i, j, k), gridX, gridY, gridZ);

    if (i-1 >= 0) {
        int leftCell = coordToCell(ivec3(i-1, j, k), gridX, gridY, gridZ);
        int cellX = coordToCell(ivec3(i, j, k), gridX+1, gridY, gridZ);
        velX[cellX] -= tensionDv(cell, leftCell) + velX[cellX] * dt * 0.01;
    }
    if (j-1 >= 0) {
        int bottomCell = coordToCell(ivec3(i, j-1, k), gridX, gridY, gridZ);
        int cellY = coordToCell(ivec3(i, j, k), gridX, gridY+1, gridZ);
        velY[cellY] -= tensionDv(cell, bottomCell) + velY[cellY] * dt * 0.01;
    }
    if (k-1 >= 0) {
        int backCell = coordToCell(ivec3(i, j, k-1), gridX, gridY, gridZ);
        int cellZ = coordToCell(ivec3(i, j, k), gridX, gridY, gridZ+1);
        velZ[cellZ] -= tensionDv(cell, backCell) + velZ[cellZ] * dt * 0.01;
    }
}
