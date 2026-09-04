#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout(std430, binding = 0) readonly buffer DataBuffer { float[] data; };
layout(std430, binding = 1) writeonly buffer GradBuffer { vec4[] grad; };

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

    int left = coordToCell(ivec3(max(i-1, 0), j, k), gridX, gridY, gridZ);
    int right = coordToCell(ivec3(min(i+1, gridX-1), j, k), gridX, gridY, gridZ);
    int bottom = coordToCell(ivec3(i, max(j-1, 0), k), gridX, gridY, gridZ);
    int top = coordToCell(ivec3(i, min(j+1, gridY-1), k), gridX, gridY, gridZ);
    int back = coordToCell(ivec3(i, j, max(k-1, 0)), gridX, gridY, gridZ);
    int front = coordToCell(ivec3(i, j, min(k+1, gridZ-1)), gridX, gridY, gridZ);
    
    float hX = float(i-1 >= 0 && i+1 < gridX);
    float hY = float(j-1 >= 0 && j+1 < gridY);
    float hZ = float(k-1 >= 0 && k+1 < gridZ);

    vec3 gradVal = vec3(data[right] - data[left], data[top] - data[bottom], data[front] - data[back]);
    gradVal /= vec3(h + hX, h + hY, h + hZ);
    grad[cell].xyz = gradVal; 
}