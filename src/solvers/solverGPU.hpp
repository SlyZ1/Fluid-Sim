#ifndef SOLVER_GPU_HPP
#define SOLVER_GPU_HPP

#include <vector>
#include <glm/glm.hpp>

#include "../shader_program.hpp"
#include "../helpers/metrics.hpp"
#include "../helpers/stats.hpp"
#include "../cgs/cgs.hpp"

class SolverGPU : public IStatsProvider {
private:
    CGS m_cgs = {};

    // FLIP Buffers
    GLuint m_partPosBuffer = 0;
    GLuint m_oldPartPosBuffer = 0;
    GLuint m_partVelBuffer = 0;
    GLuint m_velXBuffer = 0;
    GLuint m_velYBuffer = 0;
    GLuint m_velZBuffer = 0;
    GLuint m_oldVelXBuffer = 0;
    GLuint m_oldVelYBuffer = 0;
    GLuint m_oldVelZBuffer = 0;
    GLuint m_rXBuffer = 0;
    GLuint m_rYBuffer = 0;
    GLuint m_rZBuffer = 0;
    GLuint m_isAirBuffer = 0;

    // Density
    GLuint m_rhoBuffer = 0;
    GLuint m_smoothRhoBuffer = 0;
    GLuint m_gradRhoBuffer = 0;
    GLuint m_curvatureBuffer = 0;

    // Pressure - Incompression
    GLuint m_minusDivBuffer = 0;
    GLuint m_pressureBuffer = 0;

    // Particle Sort
    GLuint m_cellOfBuffer = 0;
    GLuint m_blockSumBuffer = 0;
    GLuint m_cellParticleIdsBuffer = 0;
    GLuint m_firstCellParticleBuffer = 0;
    GLuint m_firstCellParticleBuffer2 = 0;

    ShaderProgram m_integrateShader = {};
    ShaderProgram m_integrateGridShader = {};

    ShaderProgram m_collisionShader = {};

    // Particle Sort
    ShaderProgram m_resetUintBuffersShader = {};
    ShaderProgram m_partCountShader = {};
    ShaderProgram m_localSumShader = {};
    ShaderProgram m_smallSumShader = {};
    ShaderProgram m_globalSumShader = {};
    ShaderProgram m_cellParticleIdShader = {};

    // Push appart
    ShaderProgram m_resetBuffersShader = {};
    ShaderProgram m_getCorrectionsShader = {};
    ShaderProgram m_applyCorrectionsShader = {};

    // P2G
    ShaderProgram m_resetFloatBufferShader = {};
    ShaderProgram m_p2gShader = {};
    ShaderProgram m_applyWeightsShader = {};

    // Density
    ShaderProgram m_computeRhoShader = {};
    ShaderProgram m_smoothDataShader = {};
    ShaderProgram m_computeGradShader = {};
    ShaderProgram m_computeCurvatureShader = {};
    
    // Pressure - Incompression
    ShaderProgram m_solveIncompressibilityShader = {};
    ShaderProgram m_sparseMatVecShader = {};
    ShaderProgram m_computeMinusDivShader = {};
    ShaderProgram m_pressureToVelShader = {};
    ShaderProgram m_setAirCellsToZeroShader = {};
    
    ShaderProgram m_g2pShader = {};

    GPUTimer m_integrateTimer = {};
    GPUTimer m_pushAppartTimer = {};
    GPUTimer m_collisionTimer = {};
    GPUTimer m_p2gTimer = {};
    GPUTimer m_surfaceTensionTimer = {};
    GPUTimer m_incompressibilityTimer = {};
    GPUTimer m_g2pTimer = {};
    GPUTimer m_scanTimer = {};
    StatIndex m_integrationStatIndex = 0;
    StatIndex m_pushAppartStatIndex = 0;
    StatIndex m_collisionStatIndex = 0;
    StatIndex m_p2gStatIndex = 0;
    StatIndex m_surfaceTensionStatIndex = 0;
    StatIndex m_incompressibilityStatIndex = 0;
    StatIndex m_g2pStatIndex = 0;

    int m_partN = 0;

    glm::vec2 m_obstaclePos = glm::vec2(0.f);
    glm::vec2 m_obstacleVel = glm::vec2(0.f);
    float m_obstacleRadius = 0.f;

    float m_radius = 0.f;
    float m_h = 0.f;
    int m_gridX = 0;
    int m_gridY = 0;
    int m_gridZ = 0;
    
    std::vector<std::vector<int>> m_particlesInGrid = {};

    float m_dt = 0.f;

    glm::ivec3 cellToCoord(int cell, int nx, int ny);
    glm::vec3 cellToPos(int cell, int nx, int ny, int nz);

    static void loadCompute(ShaderProgram& prog, const std::string path);
 
    void deleteBuffers();
    void createBuffers();

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
    ~SolverGPU() override;

    SolverGPU(const SolverGPU&) = delete;
    SolverGPU& operator=(const SolverGPU&) = delete;

    void updateFlip();
    void updateObstacle(glm::vec2 pos, glm::vec2 vel, float radius);
    void reload();

    void setDt(float newDt) { m_dt = newDt; };

    GLuint getPosBuffer() const { return m_partPosBuffer; };
    GLuint getVelBuffer() const { return m_partVelBuffer; };
};

#endif