#version 430 core
layout(local_size_x = 256) in;

layout(std430, binding = 0) buffer PartPosBuffer { vec4[] partPos; };
layout(std430, binding = 1) buffer PartVelBuffer { vec4[] partVel; };

uniform int partN;

uniform vec3 minPos;
uniform vec3 maxPos;

uniform float radius;

uniform vec2 obstaclePos;
uniform vec2 obstacleVel;
uniform float obstacleRadius;

void main(){
    int i = int(gl_GlobalInvocationID.x);
    if (i >= partN) return;


    // vec3 diff = pos - obstaclePos;
    // float dist2 = dot(diff, diff);
    // if (dist2 < (radius + obstacleRadius) * (radius + obstacleRadius)){
    //     float d = sqrt(dist2);
    //     vec3 dir = diff / d;
    //     //partPos[i] += (radius + obstacleRadius - d) * dir;
    //     vec3 vel = partVel[i];
    //     partVel[i] = obstacleVel;
    // }

    vec3 pos = partPos[i].xyz;
    partPos[i] = vec4(clamp(pos, minPos + 1e-2f, maxPos - 1e-2f), 1);
    partVel[i] *= vec4(step(minPos, pos) * (1 - step(maxPos, pos)), 1);

    
}