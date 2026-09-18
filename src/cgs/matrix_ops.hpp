#ifndef MAT_OPT_HPP
#define MAT_OPT_HPP

#include <glad/glad.h>
#include "../shader_program.hpp"
#include <glm/glm.hpp>

class MatOps {
private:
    GLuint m_alphaBuffer = 0;
    GLuint m_partialDotBuffer = 0;

    int m_partialDotCapacity = 0;
    
    ShaderProgram m_mulProg = {};
    ShaderProgram m_transposeProg = {};
    ShaderProgram m_matVecProg = {};
    ShaderProgram m_saxpyProg = {};
    ShaderProgram m_dot1Prog = {};
    ShaderProgram m_dot2Prog = {};
    ShaderProgram m_resetScalarProg = {};
    ShaderProgram m_copyProg = {};

    void ensurePartialCapacity(int N);
public:
    MatOps();
    ~MatOps();

    MatOps(const MatOps&) = delete;
    MatOps& operator=(const MatOps&) = delete;

    void multiply(GLuint bufferA, GLuint bufferB, GLuint bufferResult, int N, bool dispatch = true);
    void dot(GLuint bufferA, GLuint bufferB, GLuint bufferResult, int N, int index = 0);
    void dot(GLuint bufferA, GLuint bufferB, GLuint bufferResult, int N, glm::ivec3 indicies);
    void dotIndirect(GLuint bufferA, GLuint bufferB, GLuint bufferResult, GLuint indirectBuffer, int offset1, int offset2, int N, int index = 0);
    void saxpy(GLuint bufferX, GLuint bufferY, GLuint bufferZ, 
               GLuint bufferAlpha, int N, std::vector<int> alphaOperations, bool dispatch = true);
    void saxpy(GLuint bufferX, GLuint bufferY, GLuint bufferZ, float alpha, int N, bool dispatch = true);
    void copy(GLuint bufferX, GLuint bufferY, int N, bool dispatch = true);
    void matVec(GLuint bufferA, GLuint bufferV, GLuint bufferResult, int N, bool dispatch = true);
    void transpose(GLuint bufferA, GLuint bufferResult, int N, bool dispatch = true);
};

#endif