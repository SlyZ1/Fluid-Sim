#ifndef SOLVER_GPU_HPP
#define SOLVER_GPU_HPP

#include "shader_program.hpp"
#include <vector>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include <iostream>
#include "helpers/utils.hpp"
#include "helpers/stats.hpp"
#include "cgs/cgs.hpp"

class SolverGPU {
private:
    CGS cgs = {};

    // FLIP Buffers
    GLuint partPosBuffer = 0;
    GLuint partVelBuffer = 0;
    GLuint velXBuffer = 0;
    GLuint velYBuffer = 0;
    GLuint velZBuffer = 0;
    GLuint oldVelXBuffer = 0;
    GLuint oldVelYBuffer = 0;
    GLuint oldVelZBuffer = 0;
    GLuint rXBuffer = 0;
    GLuint rYBuffer = 0;
    GLuint rZBuffer = 0;
    GLuint isAirBuffer = 0;

    // Density
    GLuint rhoBuffer;
    GLuint smoothRhoBuffer;
    GLuint gradRhoBuffer;
    GLuint curvatureBuffer;

    // Pressure - Incompression
    GLuint minusDivBuffer = 0;
    GLuint pressureBuffer = 0;

    // Particle Sort
    GLuint cellOfBuffer = 0;
    GLuint blockSumBuffer = 0;
    GLuint cellParticleIdsBuffer = 0;
    GLuint firstCellParticleBuffer = 0;
    GLuint firstCellParticleBuffer2 = 0;

    // Push appart
    GLuint correctionBuffer;
    GLuint numCorrectionBuffer;

    ShaderProgram integrateShader = {};
    ShaderProgram integrateGridShader = {};

    ShaderProgram collisionShader = {};

    // Particle Sort
    ShaderProgram resetUintBuffersShader = {};
    ShaderProgram partCountShader = {};
    ShaderProgram localSumShader = {};
    ShaderProgram smallSumShader = {};
    ShaderProgram globalSumShader = {};
    ShaderProgram cellParticleIdShader = {};

    // Push appart
    ShaderProgram resetBuffersShader = {};
    ShaderProgram getCorrectionsShader = {};
    ShaderProgram applyCorrectionsShader = {};

    // P2G
    ShaderProgram resetFloatBufferShader = {};
    ShaderProgram p2gShader = {};
    ShaderProgram applyWeightsShader = {};

    // Density
    ShaderProgram computeRhoShader = {};
    ShaderProgram smoothDataShader = {};
    ShaderProgram computeGradShader = {};
    ShaderProgram computeCurvatureShader = {};
    
    // Pressure - Incompression
    ShaderProgram solveIncompressibilityShader = {};
    ShaderProgram sparseMatVecShader = {};
    ShaderProgram computeMinusDivShader = {};
    ShaderProgram pressureToVelShader = {};
    ShaderProgram setAirCellsToZeroShader = {};
    
    ShaderProgram g2pShader = {};

    GPUTimer integrateTimer = {};
    GPUTimer pushAppartTimer = {};
    GPUTimer collisionTimer = {};
    GPUTimer p2gTimer = {};
    GPUTimer surfaceTensionTimer = {};
    GPUTimer incompressibilityTimer = {};
    GPUTimer g2pTimer = {};
    GPUTimer scanTimer = {};

    int partN = 0;

    vec2 obstaclePos = vec2(0.f);
    vec2 obstacleVel = vec2(0.f);
    float obstacleRadius = 0.f;

    float radius = 0.f;
    float h = 0.f;
    int gridX = 0;
    int gridY = 0;
    int gridZ = 0;
    
    vector<vector<int>> particlesInGrid = {};

    float dt = 0.f;

    ivec3 cellToCoord(int cell, int nx, int ny);
    vec3 cellToPos(int cell, int nx, int ny, int nz);

    void resetFloatBuffer(GLuint buffer, int n);
    void resetUintBuffer(GLuint buffer, int n);
    void prefixSum(GLuint data, GLuint blockSum, int n);
    void countingSort();

    void integrateParticles();
    void pushAppartParticles(int iterations);
    void particleCollisions();
    void particlesToGrid();
    void surfaceTension();
    void solveIncompressibility(int iterations, bool useCGS = true);
    void gridToParticles();

public:
    SolverGPU(int partN, float radius, float h, int gridX, int gridY, int gridZ, float timestep = 0.015f);

    void updateFlip();
    void updateObstacle(vec2 pos, vec2 vel, float radius);
    void printTimers();
    void createBuffers();
    void reload();

    void setDt(float newDt) { dt = newDt; };

    GLuint getPosBuffer() const { return partPosBuffer; };
    GLuint getVelBuffer() const { return partVelBuffer; };
};

#endif