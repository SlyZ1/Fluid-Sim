#version 430 core
layout(local_size_x = 8, local_size_y) in;

layout(std430, binding = 0) buffer PartPosBuffer { vec2[] partPos; };
layout(std430, binding = 1) buffer PartVelBuffer { vec2[] partVel; };
layout(std430, binding = 2) buffer RXBuffer { float[] rX; };
layout(std430, binding = 3) buffer RYBuffer { float[] rY; };
layout(std430, binding = 4) buffer IsAirBuffer { uint[] isAir; };

uniform int partN;
uniform int gridX;
uniform int gridY;

#pragma include "./utils.glsl"

void main(){
    int x = int(gl_GlobalInvocationID.x);
    int y = int(gl_GlobalInvocationID.y);
    if (x >= gridX || y >= gridY) return;
}