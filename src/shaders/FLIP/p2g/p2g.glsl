#version 430 core
layout(local_size_x = 8, local_size_y = 8) in;

layout(std430, binding = 0) buffer PartPosBuffer { vec2[] partPos; };
layout(std430, binding = 1) buffer PartVelBuffer { vec2[] partVel; };
layout(std430, binding = 2) buffer RXBuffer { float[] rX; };
layout(std430, binding = 3) buffer RYBuffer { float[] rY; };
layout(std430, binding = 4) buffer VelXBuffer { float[] velX; };
layout(std430, binding = 5) buffer VelYBuffer { float[] velY; };
layout(std430, binding = 6) writeonly buffer IsAirBuffer { uint[] isAir; };
layout(std430, binding = 7) readonly buffer FirstCellParticleBuffer { uint[] firstCellParticle; };
layout(std430, binding = 8) readonly buffer CellParticleIdsBuffer { uint[] cellParticleIds; };

uniform int partN;
uniform int gridX;
uniform int gridY;
uniform float h;

#pragma include "../utils.glsl"

void main(){
    int x = int(gl_GlobalInvocationID.x);
    int y = int(gl_GlobalInvocationID.y);
    if (x >= gridX || y >= gridY) return;

    ivec2 cellCoord = ivec2(x, y);
    int cell = coordToCell(cellCoord, gridX, gridY);
    int cellX = coordToCell(cellCoord, gridX+1, gridY);
    int cellY = coordToCell(cellCoord, gridX, gridY+1);
    vec2 cellCenter = coordToPos(cellCoord, gridX, gridY);
    vec2 cellCenterX = cellCenter - vec2(h * 0.5, 0);
    vec2 cellCenterY = cellCenter - vec2(0, h * 0.5);
    
    rX[cellX] = 0;
    rY[cellY] = 0;
    velX[cellX] = 0;
    velY[cellY] = 0;
    isAir[cell] = 1u;
    bool hasSetIsAirToFalse = false;

    for(int offsetX = -1; offsetX <= 1; offsetX++) {
        for(int offsetY = -1; offsetY <= 1; offsetY++)
        {
            int cx = x + offsetX;
            int cy = y + offsetY;
            if (cx < 0 || cx >= gridX || cy < 0 || cy >= gridY) continue;

            int newCell = coordToCell(ivec2(cx, cy), gridX, gridY);
            int rangeStart = int(firstCellParticle[newCell]);
            int rangeEnd = int(firstCellParticle[newCell+1]);

            for (int k = rangeStart; k < rangeEnd; k++)
            {
                if (!hasSetIsAirToFalse && offsetX == 0 && offsetY == 0){
                    isAir[cell] = 0u;
                    hasSetIsAirToFalse = true;
                }
                int i = int(cellParticleIds[k]);
                vec2 pos = partPos[i];
                vec2 vel = partVel[i];
                {
                    int cell = posToCell(pos - vec2(0, h * 0.5), gridX, gridY);
                    vec2 dp = min(abs(pos - cellCenterX) / h, 1.0);
                    float w = (1.0 - dp.x) * (1.0 - dp.y);
                    
                    velX[cellX] += w * vel.x;
                    rX[cellX] += w;
                }
                {
                    int cell = posToCell(pos - vec2(h * 0.5, 0), gridX, gridY);
                    vec2 dp = min(abs(pos - cellCenterY) / h, 1.0);
                    float w = (1.0 - dp.x) * (1.0 - dp.y);
                    
                    velY[cellY] += w * vel.y;
                    rY[cellY] += w;
                }
            }
        }   
    }
}