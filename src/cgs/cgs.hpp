#ifndef CGS_HPP
#define CGS_HPP

#include <vector>
#include <functional>
#include <glad/glad.h>
#include <memory>

#include "matrix_ops.hpp"
#include "../shader_program.hpp"

struct DispatchParams {
    GLuint num_groups_x;
    GLuint num_groups_y;
    GLuint num_groups_z;
};

class CGS {
private:
    int RTR_SCALAR_INDEX = 0;
    int RTR_NEW_SCALAR_INDEX = 1;
    static constexpr int DTAD_SCALAR_INDEX = 2;
    static constexpr int RTR0_VALUE_INDEX = 3;
    static constexpr int MINUS_ONE_SCALAR_INDEX = 9;
    static constexpr int NUM_SCALARS = 10;

    int m_n = 0;
    GLuint m_ABuffer = 0;
    GLuint m_bBuffer = 0;
    GLuint m_ATBuffer = 0;
    GLuint m_ATbBuffer = 0;
    GLuint m_ATABuffer = 0;
    
    GLuint m_zeroBuffer = 0;
    GLuint m_xBuffer = 0;
    GLuint m_dBuffer = 0;
    GLuint m_AdBuffer = 0;
    GLuint m_rBuffer = 0;
    GLuint m_scalarBuffer = 0;
    GLuint m_indirectBuffer = 0;
    std::vector<DispatchParams> m_dispatchParams = {};
    ShaderProgram m_stopProg = {};

    std::unique_ptr<MatOps> m_matOps;

    void initBuffers();
    void deleteBuffers();
    void swap_rtr_indices();

public:
    CGS();
    ~CGS();

    CGS(const CGS&) = delete;
    CGS& operator=(const CGS&) = delete;

    void compute_ATA_ATb(GLuint AMatrixbuffer, GLuint bMatrixBuffer);
    void init(int n, GLuint ABuffer, GLuint bBuffer, GLuint xBuffer);
    void reloadArgs(GLuint ABuffer, GLuint bBuffer, GLuint xBuffer);
    GLuint solve(int maxIter, float tol);
    GLuint solve(int maxIter, float tol, std::function<void(GLuint, GLuint, GLuint, int, bool)> matVec, DispatchParams matVecParams, bool reuse=true);
};

#endif