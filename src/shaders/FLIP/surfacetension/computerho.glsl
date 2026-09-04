#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout(std430, binding = 0) readonly buffer RXBuffer { float[] rX; };
layout(std430, binding = 1) readonly buffer RYBuffer { float[] rY; };
layout(std430, binding = 2) readonly buffer RZBuffer { float[] rZ; };
layout(std430, binding = 3) readonly buffer IsAirBuffer { uint[] isAir; };
layout(std430, binding = 4) writeonly buffer RhoBuffer { float[] rho; };

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
    int cell = coordToCell(ivec3(i, j, k), gridX, gridY, gridZ);

    float s0 = float(j-1 >= 0);
    float s1 = float(j+1 < gridY);
    float s2 = float(i-1 >= 0);
    float s3 = float(i+1 < gridX);
    float s4 = float(k-1 >= 0);
    float s5 = float(k+1 < gridZ);
    float s = s0 + s1 + s2 + s3 + s4 + s5;

    int bottom = coordToCell(ivec3(i, j, k), gridX, gridY+1, gridZ);
    int top = coordToCell(ivec3(i, j+1, k), gridX, gridY+1, gridZ);
    int left = coordToCell(ivec3(i, j, k), gridX+1, gridY, gridZ);
    int right = coordToCell(ivec3(i+1, j, k), gridX+1, gridY, gridZ);
    int back = coordToCell(ivec3(i, j, k), gridX, gridY, gridZ+1);
    int front = coordToCell(ivec3(i, j, k+1), gridX, gridY, gridZ+1);
    
    float r = s2 * rX[left] + s3 * rX[right];
    r += s0 * rY[bottom] + s1 * rY[top];
    r += s4 * rZ[back] + s5 * rZ[front];
    if (s > 0) r /= s;
    
    rho[cell] = r;
}