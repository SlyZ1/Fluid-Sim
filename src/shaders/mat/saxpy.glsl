#version 430 core
layout(local_size_x = 256) in;

layout(std430, binding = 0) readonly buffer Xbuffer {float X[];};
layout(std430, binding = 1) readonly buffer Ybuffer {float Y[];};
layout(std430, binding = 2) writeonly buffer Zbuffer {float Z[];};
layout(std430, binding = 3) readonly buffer ALPHA {float alphas[];};

uniform int N;
uniform int alpha_operations[16];


float intPow(float base, int exp) {
    if (exp == 0) return 1.0;
    float result = 1.0;
    float b = (exp < 0) ? 1.0 / base : base;
    int e = abs(exp);
    for (int k = 0; k < e; k++)
        result *= b;
    return result;
}

void main(){
    uint i = gl_GlobalInvocationID.x;
    if (i >= N) return;
    
    float xi = X[i];
    float yi = Y[i];
    
    float alpha = 1.0;
    for(int j = 0; j < 16; j++) {
        int operation = alpha_operations[j];
        float alpha_j = alphas[j];
        if (operation < 0 && abs(alpha_j) < 1e-10){
            alpha = 0.0;
            break;
        }
        if (operation != 0 && alpha_j < 0){
            alpha *= intPow(alpha_j, int(operation));
        }
        else if (operation != 0) alpha *= pow(alpha_j, float(operation));
    }

    Z[i] = xi + alpha * yi;
}