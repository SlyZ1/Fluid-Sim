#version 430 core
layout(local_size_x = 256) in;

layout(std430, binding = 0) writeonly buffer Buf { float data[]; };
uniform int n;

void main() {
    uint i = gl_GlobalInvocationID.x;
    if (i >= n) return;
    data[i] = 0;
}