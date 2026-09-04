#ifndef CGS_HPP
#define CGS_HPP

#include <glad/glad.h>
#include <vector>
#include "../shader_program.hpp"
#include "matrix_ops.hpp"
#include <functional>

struct DispatchParams {
    GLuint num_groups_x;
    GLuint num_groups_y;
    GLuint num_groups_z;
};

class CGS {
private:
    int RTR_SCALAR_INDEX = 0;
    int RTR_NEW_SCALAR_INDEX = 1;
    const int DTAD_SCALAR_INDEX = 2;
    const int RTR0_VALUE_INDEX = 3;
    const int MINUS_ONE_SCALAR_INDEX = 9;
    const int NUM_SCALARS = 10;
    bool tmp = false;

    int n;
    GLuint ABuffer = 0;
    GLuint bBuffer = 0;
    GLuint ATBuffer = 0;
    GLuint ATbBuffer = 0;
    GLuint ATABuffer = 0;
    
    GLuint zeroBuffer = 0;
    GLuint xBuffer = 0;
    GLuint dBuffer = 0;
    GLuint AdBuffer = 0;
    GLuint rBuffer = 0;
    GLuint scalarBuffer = 0;
    GLuint indirectBuffer = 0;
    vector<DispatchParams> dispatchParams = {};
    ShaderProgram stopProg = {};

    MatOps* matOps;

    void initBuffers();
    void swap_rtr_indices();

public:
    CGS(){};
    void compute_ATA_ATb(GLuint AMatrixbuffer, GLuint bMatrixBuffer);
    void init(int n, GLuint ABuffer, GLuint bBuffer, GLuint xBuffer);
    GLuint solve(int maxIter, float tol);
    GLuint solve(int maxIter, float tol, function<void(GLuint, GLuint, GLuint, int, bool)> matVec, DispatchParams matVecParams);
    GLuint solve(GLuint previousXBuffer, int maxIter, float tol, function<void(GLuint, GLuint, GLuint, int, bool)> matVec, DispatchParams matVecParams);
};

#endif