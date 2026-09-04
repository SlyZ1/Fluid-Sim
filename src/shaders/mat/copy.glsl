#version 430 core
layout(local_size_x = 256) in;

layout(std430, binding = 0) readonly buffer bufferX {float X[];};
layout(std430, binding = 1) writeonly buffer bufferY {float Y[];};

uniform int N;

void main(){
    uint i = gl_GlobalInvocationID.x;
    if (i >= N) return;
    Y[i] = X[i];
}