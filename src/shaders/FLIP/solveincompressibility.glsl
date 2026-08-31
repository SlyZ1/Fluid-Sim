#version 430 core
layout(local_size_x = 8, local_size_y = 8) in;

layout(std430, binding = 0) readonly buffer RXBuffer { float[] rX; };
layout(std430, binding = 1) readonly buffer RYBuffer { float[] rY; };
layout(std430, binding = 2) buffer VelXBuffer { float[] velX; };
layout(std430, binding = 3) buffer VelYBuffer { float[] velY; };
layout(std430, binding = 4) buffer IsAirBuffer { uint[] isAir; };

uniform int gridX;
uniform int gridY;
uniform float h;
uniform int isPair;

#pragma include "./utils.glsl"

void main(){
    int i = int(gl_GlobalInvocationID.x);
    int j = int(gl_GlobalInvocationID.y);
    if ((i + j) % 2 == 1 - isPair || i >= gridX || j >= gridY) return;
    if (isAir[coordToCell(ivec2(i,j), gridX, gridY)] == 1u) return;

    float s0 = float(j-1 >= 0);
    float s1 = float(j+1 < gridY);
    float s2 = float(i-1 >= 0);
    float s3 = float(i+1 < gridX);
    float s = s0 + s1 + s2 + s3;
    if (s <= 0) return;

    int bottom = coordToCell(ivec2(i, j), gridX, gridY+1);
    int top = coordToCell(ivec2(i, j+1), gridX, gridY+1);
    int left = coordToCell(ivec2(i, j), gridX+1, gridY);
    int right = coordToCell(ivec2(i+1, j), gridX+1, gridY);

    float velBottom = velY[bottom];
    float velTop = velY[top];
    float velLeft = velX[left];
    float velRight = velX[right];

    float d = s1 * velTop - s0 * velBottom;
    d += s3 * velRight - s2 * velLeft;
    d *= 1.9f; // overrelaxation
    
    float rho = s2 * rX[left] + s3 * rX[right];
    rho += s0 * rY[bottom] + s1 * rY[top];
    rho /= s;
    d -= max(rho - 1.5, 0); // drift compensation
    
    velY[bottom] = velBottom + d * s0 / s;
    velY[top] = velTop - d * s1 / s;
    velX[left] = velLeft + d * s2 / s;
    velX[right] = velRight - d * s3 / s;
}