#version 430 core
layout(local_size_x = 16, local_size_y = 16) in;

layout(std430, binding = 0) readonly buffer MatA { float A[]; };
layout(std430, binding = 1) readonly buffer MatB { float B[]; };
layout(std430, binding = 2) writeonly buffer MatC { float C[]; };

uniform int N;

shared float tileA[16][16];
shared float tileB[16][16];

void main() {
    uint row = gl_GlobalInvocationID.y;
    uint col = gl_GlobalInvocationID.x;
    uint X = gl_LocalInvocationID.x;
    uint Y = gl_LocalInvocationID.y;
    
    float sum = 0.0;
    
    for (int t = 0; t < (N + 15) / 16; t++) {
        // Load data in shared mem
        tileA[Y][X] = 
            (row < N && t * 16 + X < N) 
            ? A[row * N + t * 16 + X] 
            : 0.0;
        tileB[Y][X] = 
            ((t * 16 + Y) < N && col < N) 
            ? B[(t * 16 + Y) * N + col] 
            : 0.0;
        
        barrier();
        
        // Compute product
        for (int k = 0; k < 16; k++)
            sum += tileA[Y][k] * tileB[k][X];
        
        barrier();
    }
    
    if (row < N && col < N)
        C[row * N + col] = sum;
}