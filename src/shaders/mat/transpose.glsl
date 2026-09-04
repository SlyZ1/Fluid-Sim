#version 430 core
layout(local_size_x = 16, local_size_y = 16) in;

layout(std430, binding = 0) readonly buffer InputA {
    float A[];
};
layout(std430, binding = 1) writeonly buffer OutputA {
    float A_T[];
};

uniform int N;

void main() {
    uint i = gl_GlobalInvocationID.x;
    uint j = gl_GlobalInvocationID.y;
    if (i >= N || j >= N) return;
    A_T[i * N + j] = A[j * N + i];
}