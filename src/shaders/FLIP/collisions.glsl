#version 430 core
layout(local_size_x = 256) in;

layout(std430, binding = 0) buffer PartPosBuffer { vec2[] partPos; };
layout(std430, binding = 1) buffer PartVelBuffer { vec2[] partVel; };

uniform int partN;

uniform vec2 minPos;
uniform vec2 maxPos;

uniform float radius;

uniform vec2 obstaclePos;
uniform vec2 obstacleVel;
uniform float obstacleRadius;

void main(){
    int i = int(gl_GlobalInvocationID.x);
    if (i >= partN) return;

    vec2 pos = partPos[i];

    vec2 diff = pos - obstaclePos;
    float dist2 = dot(diff, diff);
    if (dist2 < (radius + obstacleRadius) * (radius + obstacleRadius)){
        float d = sqrt(dist2);
        vec2 dir = diff / d;
        partPos[i] += (radius + obstacleRadius - d) * dir;
        partVel[i] = partVel[i] - max(0., dot(partVel[i], dir)) * dir + obstacleVel;
    }

    if (pos.x < minPos.x + 1e-2f){
        partPos[i].x = minPos.x + 1e-2f;
        partVel[i].x = 0.f;
    }
    else if (pos.x > maxPos.x - 1e-2f){
        partPos[i].x = maxPos.x - 1e-2f;
        partVel[i].x = 0.f;
    }
    if (pos.y < minPos.y + 1e-2f){
        partPos[i].y = minPos.y + 1e-2f;
        partVel[i].y = 0.f;
    }
    else if (pos.y > maxPos.y - 1e-2f){
        partPos[i].y = maxPos.y - 1e-2f;
        partVel[i].y = 0.f;
    }
}