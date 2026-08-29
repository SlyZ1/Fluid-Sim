#ifndef SOLVER_GPU_HPP
#define SOLVER_GPU_HPP

#include "shader_program.hpp"
#include <vector>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include <iostream>
#include "helpers/utils.hpp"

class SolverGPU {
private:
    GLuint partPosBuffer = 0;
    GLuint partVelBuffer = 0;
    GLuint velXBuffer = 0;
    GLuint velYBuffer = 0;
    GLuint rXBuffer = 0;
    GLuint rYBuffer = 0;

    GLuint cellOfBuffer = 0;
    GLuint blockSumBuffer = 0;
    GLuint cellParticleIdsBuffer = 0;
    GLuint firstCellParticleBuffer = 0;

    ShaderProgram integrateShader = {};

    ShaderProgram collisionShader = {};

    ShaderProgram resetUintBuffersShader = {};
    ShaderProgram partCountShader = {};
    ShaderProgram localSumShader = {};
    ShaderProgram smallSumShader = {};
    ShaderProgram globalSumShader = {};
    ShaderProgram resetBuffersShader = {};
    ShaderProgram getCorrectionsShader = {};
    ShaderProgram applyCorrectionsShader = {};

    int partN = 0;

    vec2 obstaclePos = vec2(0.f);
    vec2 obstacleVel = vec2(0.f);
    float obstacleRadius = 0.f;

    float radius = 0.f;
    float h = 0.f;
    int gridX = 0;
    int gridY = 0;
    
    vector<vector<int>> particlesInGrid = {};

    float dt = 0.f;

    ivec2 cellToCoord(int cell, int nx);
    vec2 cellToPos(int cell, int nx, int ny);

    void resetUintBuffer(GLuint buffer, int n);
    void prefixSum(GLuint data, GLuint blockSum, int n);

    void integrateParticles();
    void pushAppartParticles();
    void particleCollisions();
    void particlesToGrid();
    void solveIncompressibility(int iterations);
    void gridToParticles();

public:
    SolverGPU(int partN, float radius, float h, int gridX, int gridY, float timestep = 0.015f);

    void updateFlip();
    void updateObstacle(vec2 pos, vec2 vel, float radius);

    GLuint getPosBuffer() const { return partPosBuffer; };
    GLuint getVelBuffer() const { return partVelBuffer; };
};

#endif