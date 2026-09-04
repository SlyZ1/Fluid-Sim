#version 430 core
layout(local_size_x = 1) in;
layout(std430, binding = 0) buffer ScalarBuffer { uint scalars[]; };
uniform int index;

void main() {
    scalars[index] = 0u;
}