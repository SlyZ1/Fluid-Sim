#version 430 core
layout(local_size_x = 256) in;

// Blelloch method

shared uint temp[512];

layout(std430, binding = 0) buffer DataBuffer { uint[] data; }; // data.size() = k * 512 !!!!!!!!!
layout(std430, binding = 1) writeonly buffer BlockSumBuffer { uint[] blockSum; };

void main(){
    uint id = gl_LocalInvocationID.x;
    uint offset = 1;

    // Mem load
    temp[2*id] = data[2*id];
    temp[2*id+1] = data[2*id+1];

    // Reduce
    for(int d = 256; d > 0; d >>= 1) {
        barrier();
        if (id < d){
            uint ai = offset*(2*id+1)-1;
            uint bi = offset*(2*id+2)-1;
            temp[bi] += temp[ai];
        }
        offset <<= 1;
    }

    // Exclusivity
    if (id == 0) temp[511] = 0;

    // Down-Sweep
    for(uint d = 1; d <= 256; d <<= 1) {
        offset >>= 1;
        barrier();
        if (id < d){
            uint ai = offset*(2*id+1)-1;
            uint bi = offset*(2*id+2)-1;
            uint t = temp[ai];
            temp[ai] = temp[bi];
            temp[bi] += temp[ai];
        }
    }

    barrier();
    data[2*id] = temp[2*id];
    data[2*id+1] = temp[2*id+1];

    if (id == gl_WorkGroupSize.x - 1){
        blockSum[gl_WorkGroupID.x] = temp[2*id+1] + data[2*id+1];
    }
}