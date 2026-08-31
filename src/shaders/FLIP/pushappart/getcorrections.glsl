#version 430 core
layout(local_size_x = 64) in;

layout(std430, binding = 0) buffer CorrectionsBuffer { vec2[] corrections; };
layout(std430, binding = 1) buffer NumCorrectionsBuffer { int[] numCorrections; };
layout(std430, binding = 2) readonly buffer FirstCellParticleBuffer { uint[] firstCellParticle; };
layout(std430, binding = 3) readonly buffer CellParticleIdsBuffer { uint[] cellParticleIds; };
layout(std430, binding = 4) readonly buffer PartPosBuffer { vec2[] partPos; };

uniform int partN;
uniform int gridX;
uniform int gridY;
uniform float h;

uniform float minDist;

#pragma include "../utils.glsl"

void main(){
    int i = int(gl_GlobalInvocationID.x);
    if (i >= partN) return;

    ivec2 coord = posToCoord(partPos[i], gridX, gridY);

    for(int x = -1; x <= 1; x++) {
        for(int y = -1; y <= 1; y++) 
        {
            int cx = coord.x + x;
            int cy = coord.y + y;
            if (cx < 0 || cx >= gridX || cy < 0 || cy >= gridY) continue;

            int newCell = coordToCell(ivec2(cx, cy), gridX, gridY);
            int rangeStart = int(firstCellParticle[newCell]);
            int rangeEnd = int(firstCellParticle[newCell+1]);

            for (int k = rangeStart; k < rangeEnd; k++)
            {
                int j = int(cellParticleIds[k]);
                if (j == i) continue;

                vec2 diff = partPos[i] - partPos[j];
                float dist2 = dot(diff, diff);
                if (dist2 < minDist * minDist && dist2 > 0.)
                {
                    float dist = sqrt(dist2);
                    vec2 push = 0.5 * diff * (minDist - dist) / dist;
                    corrections[i] += push;
                    numCorrections[i]++;
                }
            }
        }   
    }
}