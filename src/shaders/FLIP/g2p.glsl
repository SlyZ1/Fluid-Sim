#version 430 core
layout(local_size_x = 64) in;

layout(std430, binding = 0) readonly buffer PartPosBuffer { vec2[] partPos; };
layout(std430, binding = 1) buffer PartVelBuffer { vec2[] partVel; };
layout(std430, binding = 2) readonly buffer VelXBuffer { float[] velX; };
layout(std430, binding = 3) readonly buffer VelYBuffer { float[] velY; };
layout(std430, binding = 4) readonly buffer OldVelXBuffer { float[] oldVelX; };
layout(std430, binding = 5) readonly buffer OldVelYBuffer { float[] oldVelY; };
layout(std430, binding = 6) readonly buffer IsAirBuffer { uint[] isAir; };

uniform int partN;
uniform int gridX;
uniform int gridY;
uniform float h;

#pragma include "./utils.glsl"

void main(){
    int i = int(gl_GlobalInvocationID.x);
    if (i >= partN) return;

    vec2 pos = partPos[i];
    {
        int cell = posToCell(pos - vec2(h * 0.5f, 0.f), gridX, gridY);
        ivec2 coord = cellToCoord(cell, gridX);
        vec2 cellCenter = coordToPos(coord, gridX, gridY) - vec2(0.f, h * 0.5f);
        vec2 dp = (pos - cellCenter) / h;
        
        float s1 = float(isAir[coordToCell(coord + ivec2(0,0), gridX, gridY)] == 0u || (coord.y + 1 <= gridY && isAir[coordToCell(coord + ivec2(0,1), gridX, gridY)] == 0u));
        float s2 = float(isAir[coordToCell(coord + ivec2(1,0), gridX, gridY)] == 0u || (coord.y + 1 <= gridY && isAir[coordToCell(coord + ivec2(1,1), gridX, gridY)] == 0u));
        float s3 = float(isAir[coordToCell(coord + ivec2(1,1), gridX, gridY)] == 0u || (coord.y + 2 <= gridY && isAir[coordToCell(coord + ivec2(1,2), gridX, gridY)] == 0u));
        float s4 = float(isAir[coordToCell(coord + ivec2(0,1), gridX, gridY)] == 0u || (coord.y + 2 <= gridY && isAir[coordToCell(coord + ivec2(0,2), gridX, gridY)] == 0u));

        float w1 = s1 * (1.f - dp.x) * (1.f - dp.y);
        float w2 = s2 * dp.x * (1.f - dp.y);
        float w3 = s3 * dp.x * dp.y;
        float w4 = s4 * (1.f - dp.x) * dp.y;
        float sumW = w1 + w2 + w3 + w4;
        if (sumW > 0){
            int bottomLeft = coordToCell(coord + ivec2(0, 0), gridX, gridY+1);
            int bottomRight = coordToCell(coord + ivec2(1, 0), gridX, gridY+1);
            int topRight = coordToCell(coord + ivec2(1, 1), gridX, gridY+1);
            int topLeft = coordToCell(coord + ivec2(0, 1), gridX, gridY+1);

            float pic = velY[bottomLeft] * w1 + velY[bottomRight] * w2 + velY[topRight] * w3 + velY[topLeft] * w4;
            float flip = pic - (oldVelY[bottomLeft] * w1 + oldVelY[bottomRight] * w2 + oldVelY[topRight] * w3 + oldVelY[topLeft] * w4);
            pic /= sumW;
            flip /= sumW;
            partVel[i].y += flip;
            partVel[i].y = mix(pic, partVel[i].y, 0.9);
        }
    }
    {
        int cell = posToCell(pos - vec2(0.f, h * 0.5f), gridX, gridY);
        ivec2 coord = cellToCoord(cell, gridX);
        vec2 cellCenter = coordToPos(coord, gridX, gridY) - vec2(h * 0.5f, 0.f);
        vec2 dp = (pos - cellCenter) / h;

        float s1 = float(isAir[coordToCell(coord + ivec2(0,0), gridX, gridY)] == 0u || (coord.x + 1 <= gridX && isAir[coordToCell(coord + ivec2(1,0), gridX, gridY)] == 0u));
        float s2 = float(isAir[coordToCell(coord + ivec2(1,0), gridX, gridY)] == 0u || (coord.x + 1 <= gridX && isAir[coordToCell(coord + ivec2(2,0), gridX, gridY)] == 0u));
        float s3 = float(isAir[coordToCell(coord + ivec2(1,1), gridX, gridY)] == 0u || (coord.x + 2 <= gridX && isAir[coordToCell(coord + ivec2(2,1), gridX, gridY)] == 0u));
        float s4 = float(isAir[coordToCell(coord + ivec2(0,1), gridX, gridY)] == 0u || (coord.x + 2 <= gridX && isAir[coordToCell(coord + ivec2(1,1), gridX, gridY)] == 0u));
        
        float w1 = s1 * (1.f - dp.x) * (1.f - dp.y);
        float w2 = s2 * dp.x * (1.f - dp.y);
        float w3 = s3 * dp.x * dp.y;
        float w4 = s4 * (1.f - dp.x) * dp.y;
        float sumW = w1 + w2 + w3 + w4;
        if (sumW > 0) {
            int bottomLeft = coordToCell(coord + ivec2(0, 0), gridX+1, gridY);
            int bottomRight = coordToCell(coord + ivec2(1, 0), gridX+1, gridY);
            int topRight = coordToCell(coord + ivec2(1, 1), gridX+1, gridY);
            int topLeft = coordToCell(coord + ivec2(0, 1), gridX+1, gridY);
        
            float pic = velX[bottomLeft] * w1 + velX[bottomRight] * w2 + velX[topRight] * w3 + velX[topLeft] * w4;
            float flip = pic - (oldVelX[bottomLeft] * w1 + oldVelX[bottomRight] * w2 + oldVelX[topRight] * w3 + oldVelX[topLeft] * w4);
            pic /= sumW;
            flip /= sumW;
            partVel[i].x += flip;
            partVel[i].x = mix(pic, partVel[i].x, 0.9);
        }
    }
}