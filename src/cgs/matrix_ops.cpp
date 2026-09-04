#include "matrix_ops.hpp"

MatOps::MatOps(){
    mulProg.create();
    mulProg.load(GL_COMPUTE_SHADER, "src/shaders/mat/multiply.glsl");
    mulProg.link();
    
    transposeProg.create();
    transposeProg.load(GL_COMPUTE_SHADER, "src/shaders/mat/transpose.glsl");
    transposeProg.link();
    
    matVecProg.create();
    matVecProg.load(GL_COMPUTE_SHADER, "src/shaders/mat/mat_vec.glsl");
    matVecProg.link();
    
    saxpyProg.create();
    saxpyProg.load(GL_COMPUTE_SHADER, "src/shaders/mat/saxpy.glsl");
    saxpyProg.link();
    
    dot1Prog.create();
    dot1Prog.load(GL_COMPUTE_SHADER, "src/shaders/mat/dot1.glsl");
    dot1Prog.link();
    
    dot2Prog.create();
    dot2Prog.load(GL_COMPUTE_SHADER, "src/shaders/mat/dot2.glsl");
    dot2Prog.link();

    copyProg.create();
    copyProg.load(GL_COMPUTE_SHADER, "src/shaders/mat/copy.glsl");
    copyProg.link();

    resetScalarProg.create();
    resetScalarProg.load(GL_COMPUTE_SHADER, "src/shaders/mat/reset_scalar.glsl");
    resetScalarProg.link();

    glDeleteBuffers(1, &partialDotBuffer);
    glGenBuffers(1, &partialDotBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, partialDotBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 1024 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
}

void MatOps::multiply(GLuint bufferA, GLuint bufferB, GLuint bufferResult, int N, bool dispatch){
    mulProg.use();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufferA);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufferB);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, bufferResult);
    glUniform1i(ShaderProgram::getVarLoc("N"), N);
    
    if (!dispatch) return;
    glDispatchCompute((N + 15) / 16, (N + 15) / 16, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void MatOps::dot(GLuint bufferA, GLuint bufferB, GLuint bufferResult, int N, int index){
    dot1Prog.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufferA);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufferB);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, partialDotBuffer);
    glUniform1i(ShaderProgram::getVarLoc("N"), N);

    dot1Prog.dispatch((N + 127) / 128);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    dot2Prog.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, partialDotBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufferResult);
    glUniform1i(ShaderProgram::getVarLoc("numPartials"), (N + 127) / 128);
    glUniform3i(ShaderProgram::getVarLoc("indicies"), index, -1, -1);
    dot2Prog.dispatch(1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    
    // if (!dispatch) return;
    // dot1Prog.dispatch((N + 127) / 128);
    // glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void MatOps::dot(GLuint bufferA, GLuint bufferB, GLuint bufferResult, int N, glm::ivec3 indicies){
    dot1Prog.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufferA);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufferB);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, partialDotBuffer);
    glUniform1i(ShaderProgram::getVarLoc("N"), N);

    dot1Prog.dispatch((N + 127) / 128);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    dot2Prog.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, partialDotBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufferResult);
    glUniform1i(ShaderProgram::getVarLoc("numPartials"), (N + 127) / 128);
    glUniform3i(ShaderProgram::getVarLoc("indicies"), indicies.x, indicies.y, indicies.z);
    dot2Prog.dispatch(1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    
    // if (!dispatch) return;
    // dot1Prog.dispatch((N + 127) / 128);
    // glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}


void MatOps::dotIndirect(GLuint bufferA, GLuint bufferB, GLuint bufferResult, GLuint indirectBuffer, int offset1, int offset2, int N, int index){
    dot1Prog.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufferA);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufferB);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, partialDotBuffer);
    glUniform1i(ShaderProgram::getVarLoc("N"), N);

    ShaderProgram::indirectDispatch(indirectBuffer, offset1);
    ShaderProgram::indirectBarrier();

    dot2Prog.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, partialDotBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufferResult);
    glUniform1i(ShaderProgram::getVarLoc("numPartials"), (N + 127) / 128);
    glUniform3i(ShaderProgram::getVarLoc("indicies"), index, -1, -1);

    ShaderProgram::indirectDispatch(indirectBuffer, offset2);
}


void MatOps::saxpy(GLuint bufferX, GLuint bufferY, GLuint bufferZ, 
                   GLuint bufferAlpha, int N, vector<int> alphaOperations, bool dispatch){
    saxpyProg.use();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufferX);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufferY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, bufferZ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, bufferAlpha);
    glUniform1i(ShaderProgram::getVarLoc("N"), N);
    glUniform1iv(ShaderProgram::getVarLoc("alpha_operations"), 16, alphaOperations.data());
    
    if (!dispatch) return;
    glDispatchCompute((N + 127) / 128, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void MatOps::saxpy(GLuint bufferX, GLuint bufferY, GLuint bufferZ, float alpha, int N, bool dispatch){
    saxpyProg.use();

    if (alphaBuffer == 0){
        glGenBuffers(1, &alphaBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, alphaBuffer);
    }
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float), &alpha, GL_DYNAMIC_COPY);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufferX);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufferY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, bufferZ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, alphaBuffer);
    glUniform1i(ShaderProgram::getVarLoc("N"), N);
    vector<int> alphaOperations(16, 0);
    alphaOperations[0] = 1;
    glUniform1iv(ShaderProgram::getVarLoc("alpha_operations"), 16, alphaOperations.data());
    
    if (!dispatch) return;
    glDispatchCompute((N + 127) / 128, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void MatOps::copy(GLuint bufferX, GLuint bufferY, int N, bool dispatch){
    copyProg.use();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufferX);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufferY);
    glUniform1i(ShaderProgram::getVarLoc("N"), N);

    if (!dispatch) return;
    glDispatchCompute((N + 255) / 256, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void MatOps::matVec(GLuint bufferA, GLuint bufferV, GLuint bufferResult, int N, bool dispatch){
    matVecProg.use();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufferA);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufferV);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, bufferResult);
    glUniform1i(ShaderProgram::getVarLoc("N"), N);
    
    if (!dispatch) return;
    glDispatchCompute((N + 63) / 64, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void MatOps::transpose(GLuint bufferA, GLuint bufferResult, int N, bool dispatch){
    transposeProg.use();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufferA);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufferResult);
    glUniform1i(ShaderProgram::getVarLoc("N"), N);
    
    if (!dispatch) return;
    glDispatchCompute((N + 15) / 16, (N + 15) / 16, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}