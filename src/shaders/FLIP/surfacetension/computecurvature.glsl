#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout(std430, binding = 0) readonly buffer GradBuffer { vec4[] grad; };
layout(std430, binding = 1) writeonly buffer CurvatureBuffer { float[] curvature; };

uniform int gridX;
uniform int gridY;
uniform int gridZ;
uniform float h;

#pragma include "../utils.glsl"

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

    int left = coordToCell(ivec3(max(i-1, 0), j, k), gridX, gridY, gridZ);
    int right = coordToCell(ivec3(min(i+1, gridX-1), j, k), gridX, gridY, gridZ);
    int bottom = coordToCell(ivec3(i, max(j-1, 0), k), gridX, gridY, gridZ);
    int top = coordToCell(ivec3(i, min(j+1, gridY-1), k), gridX, gridY, gridZ);
    int back = coordToCell(ivec3(i, j, max(k-1, 0)), gridX, gridY, gridZ);
    int front = coordToCell(ivec3(i, j, min(k+1, gridZ-1)), gridX, gridY, gridZ);
    
    float hX = float(i-1 >= 0 && i+1 < gridX);
    float hY = float(j-1 >= 0 && j+1 < gridY);
    float hZ = float(k-1 >= 0 && k+1 < gridZ);

    vec3 normalLeft = safeNormal(grad[left].xyz);
    vec3 normalRight = safeNormal(grad[right].xyz);
    vec3 normalBottom = safeNormal(grad[bottom].xyz);
    vec3 normalTop = safeNormal(grad[top].xyz);
    vec3 normalBack = safeNormal(grad[back].xyz);
    vec3 normalFront = safeNormal(grad[front].xyz);

    vec3 curvatureVal = vec3(normalRight.x - normalLeft.x, normalTop.y - normalBottom.y, normalFront.z - normalBack.z);
    curvatureVal /= vec3(h + hX, h + hY, h + hZ);
    curvature[cell] = curvatureVal.x + curvatureVal.y + curvatureVal.z; 
}