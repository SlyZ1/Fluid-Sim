#version 430 core
layout(local_size_x = 16, local_size_y = 16) in;

layout(std430, binding = 2) buffer RXBuffer { float[] rX; };
layout(std430, binding = 3) buffer RYBuffer { float[] rY; };
layout(std430, binding = 4) buffer VelXBuffer { float[] velX; };
layout(std430, binding = 5) buffer VelYBuffer { float[] velY; };

uniform int partN;
uniform int gridX;
uniform int gridY;
uniform float h;

#pragma include "../utils.glsl"

void main(){
    int x = int(gl_GlobalInvocationID.x);
    int y = int(gl_GlobalInvocationID.y);
    if (x > gridX || y > gridY) return;

    ivec2 coord = ivec2(x, y);

    if (y < gridY){
        int i = coordToCell(coord, gridX+1, gridY);
        float r = rX[i];
        if (x == 0 || x == gridX) velX[i] = 0; // is wall
        else if (r > 0) velX[i] /= r;
    }
    if (x < gridX){
        int i = coordToCell(coord, gridX, gridY+1);
        float r = rY[i];
        if (y == 0 || y == gridY) velY[i] = 0; // is wall
        else if (r > 0) velY[i] /= r;
    }
}