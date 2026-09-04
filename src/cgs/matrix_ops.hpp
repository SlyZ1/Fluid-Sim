#ifndef MAT_OPT_HPP
#define MAT_OPT_HPP

#include <glad/glad.h>
#include "../shader_program.hpp"
#include <glm/glm.hpp>

class MatOps {
private:
    GLuint alphaBuffer;
    GLuint partialDotBuffer;
    int partialDotCapacity = 0;
    
    void ensurePartialCapacity(int N);
public:
    MatOps();
    ShaderProgram mulProg = {};
    ShaderProgram transposeProg = {};
    ShaderProgram matVecProg = {};
    ShaderProgram saxpyProg = {};
    ShaderProgram dot1Prog = {};
    ShaderProgram dot2Prog = {};
    ShaderProgram resetScalarProg = {};
    ShaderProgram copyProg = {};
    void multiply(GLuint bufferA, GLuint bufferB, GLuint bufferResult, int N, bool dispatch = true);
    void dot(GLuint bufferA, GLuint bufferB, GLuint bufferResult, int N, int index = 0);
    void dot(GLuint bufferA, GLuint bufferB, GLuint bufferResult, int N, glm::ivec3 indicies);
    void dotIndirect(GLuint bufferA, GLuint bufferB, GLuint bufferResult, GLuint indirectBuffer, int offset1, int offset2, int N, int index = 0);
    void saxpy(GLuint bufferX, GLuint bufferY, GLuint bufferZ, 
               GLuint bufferAlpha, int N, vector<int> alphaOperations, bool dispatch = true);
    void saxpy(GLuint bufferX, GLuint bufferY, GLuint bufferZ, float alpha, int N, bool dispatch = true);
    void copy(GLuint bufferX, GLuint bufferY, int N, bool dispatch = true);
    void matVec(GLuint bufferA, GLuint bufferV, GLuint bufferResult, int N, bool dispatch = true);
    void transpose(GLuint bufferA, GLuint bufferResult, int N, bool dispatch = true);
};

#endif