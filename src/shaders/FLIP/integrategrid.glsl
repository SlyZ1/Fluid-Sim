#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout(std430, binding = 0) readonly buffer GradBuffer { vec4[] grad; };
layout(std430, binding = 1) readonly buffer CurvatureBuffer { float[] curvature; };
layout(std430, binding = 2) readonly buffer IsAirBuffer { uint[] isAir; };
layout(std430, binding = 3) buffer VelXBuffer { float[] velX; };
layout(std430, binding = 4) buffer VelYBuffer { float[] velY; };
layout(std430, binding = 5) buffer VelZBuffer { float[] velZ; };

uniform int gridX;
uniform int gridY;
uniform int gridZ;
uniform float h;
uniform float dt;
uniform float sigma;

#pragma include "./utils.glsl"

vec3 safeNormal(vec3 x){
    float len = length(x);
    return len > 1e-6 ? x / len : vec3(0);
}

void main(){
    int i = int(gl_GlobalInvocationID.x);
    int j = int(gl_GlobalInvocationID.y);
    int k = int(gl_GlobalInvocationID.z);
    if (i >= gridX || j >= gridY || k >= gridZ) return;
    int cell = coordToCell(ivec3(i, j, k), gridX, gridY, gridZ);

    vec3 normal = vec3(0.0); float curv = 0.0;
    bool hasMiddle = isAir[cell] == 0u;
    if (hasMiddle){
        normal = safeNormal(grad[cell].xyz);
        curv = curvature[cell];
    }
    if (i-1 >= 0) {
        int leftCell = coordToCell(ivec3(i-1, j, k), gridX, gridY, gridZ);
        bool hasLeft = isAir[leftCell] == 0u;

        vec3 leftNormal = vec3(0.0); float leftCurv = 0.0;
        if (hasLeft){
            leftNormal = safeNormal(grad[leftCell].xyz);
            leftCurv = curvature[leftCell];
        }

        int validCount = int(hasLeft) + int(hasMiddle);
        if (validCount > 0){
            vec3 force = sigma * (curv + leftCurv) / validCount * (normal + leftNormal) / validCount;
            int cellX = coordToCell(ivec3(i, j, k), gridX+1, gridY, gridZ);
            velX[cellX] -= force.x * dt;
        }
    }
    if (j-1 >= 0) {
        int bottomCell = coordToCell(ivec3(i, j-1, k), gridX, gridY, gridZ);
        bool hasBottom = isAir[bottomCell] == 0u;

        vec3 bottomNormal = vec3(0.0); float bottomCurv = 0.0;
        if (hasBottom){
            bottomNormal = safeNormal(grad[bottomCell].xyz);
            bottomCurv = curvature[bottomCell];
        }

        int validCount = int(hasBottom) + int(hasMiddle);
        if (validCount > 0){
            vec3 force = sigma * (curv + bottomCurv) / validCount * (normal + bottomNormal) / validCount;
            int cellY = coordToCell(ivec3(i, j, k), gridX, gridY+1, gridZ);
            velY[cellY] -= force.y * dt;
        }
    }
    if (k-1 >= 0) {
        int backCell = coordToCell(ivec3(i, j, k-1), gridX, gridY, gridZ);
        bool hasBack = isAir[backCell] == 0u;

        vec3 backNormal = vec3(0.0); float backCurv = 0.0;
        if (hasBack){
            backNormal = safeNormal(grad[backCell].xyz);
            backCurv = curvature[backCell];
        }

        int validCount = int(hasBack) + int(hasMiddle);
        if (validCount > 0){
            vec3 force = sigma * (curv + backCurv) / validCount * (normal + backNormal) / validCount;
            int cellZ = coordToCell(ivec3(i, j, k), gridX, gridY, gridZ+1);
            velZ[cellZ] -= force.z * dt;
        }
    }
}