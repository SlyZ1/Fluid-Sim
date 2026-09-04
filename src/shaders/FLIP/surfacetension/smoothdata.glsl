#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout(std430, binding = 0) readonly buffer DataBuffer { float[] data; };
layout(std430, binding = 1) writeonly buffer SmoothedDataBuffer { float[] smoothedData; };

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

    float s0 = float(i-1 >= 0);
    float s1 = float(i+1 < gridX);
    float s2 = float(j-1 >= 0);
    float s3 = float(j+1 < gridY);
    float s4 = float(k-1 >= 0);
    float s5 = float(k+1 < gridZ);
    float s = s0 + s1 + s2 + s3 + s4 + s5;

    float dataLeft = 0.0, dataRight = 0.0, dataBottom = 0.0, dataTop = 0.0, dataBack = 0.0, dataFront = 0.0;
    if (s0 > 0) {
        int left = coordToCell(ivec3(i-1, j, k), gridX, gridY, gridZ);
        dataLeft = data[left];
    }
    if (s1 > 0) {
        int right = coordToCell(ivec3(i+1, j, k), gridX, gridY, gridZ);
        dataRight = data[right];
    }
    if (s2 > 0) {
        int bottom = coordToCell(ivec3(i, j-1, k), gridX, gridY, gridZ);
        dataBottom = data[bottom];
    }
    if (s3 > 0) {
        int top = coordToCell(ivec3(i, j+1, k), gridX, gridY, gridZ);
        dataTop = data[top];
    }
    if (s4 > 0) {
        int back = coordToCell(ivec3(i, j, k-1), gridX, gridY, gridZ);
        dataBack = data[back];
    }
    if (s5 > 0) {
        int front = coordToCell(ivec3(i, j, k+1), gridX, gridY, gridZ);
        dataFront = data[front];
    }
    
    float smoothedVal = dataLeft + dataRight + dataBottom + dataTop + dataBack + dataFront;
    smoothedVal += s * data[cell];
    if (s > 0){
        smoothedData[cell] = smoothedVal / (2 * s);
    }
    else{
        smoothedData[cell] = data[cell];
    }
}