#ifndef SOLVER_HPP
#define SOLVER_HPP

#include <vector>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

class Solver{
private:
    int m_partN = 0;
    std::vector<glm::vec2> m_partPos = {};
    std::vector<glm::vec2> m_partVel = {};

    glm::vec2 m_obstaclePos = glm::vec2(0.f);
    glm::vec2 m_obstacleVel = glm::vec2(0.f);
    float m_obstacleRadius = 0.f;

    float m_radius = 0.f;
    float m_h = 0.f;
    int m_gridX = 0;
    int m_gridY = 0;
    std::vector<float> m_oldVelX = {};
    std::vector<float> m_oldVelY = {};
    std::vector<float> m_velX = {};
    std::vector<float> m_velY = {};
    std::vector<float> m_rX = {};
    std::vector<float> m_rY = {};
    std::vector<bool> m_isAir = {};
    std::vector<bool> m_isWall = {};

    std::vector<std::vector<int>> m_particlesInGrid = {};

    float m_dt = 0.f;

    glm::ivec2 cellToCoord(int cell, int nx);
    int coordToCell(glm::ivec2 coord, int nx, int ny);
    int posToCell(glm::vec2 pos, int nx, int ny);
    glm::vec2 cellToPos(int cell, int nx, int ny);
    glm::vec2 coordToPos(glm::ivec2 coord, int nx, int ny);

    void posToCoordAndDp(glm::vec2 pos, int nx, int ny, glm::ivec2& coord, glm::vec2& dp);

    void integrateParticles();
    void pushAppartParticles();
    void particleCollisions();
    void particlesToGrid();
    void solveIncompressibility(int iterations);
    void gridToParticles();

public:
    Solver(int partN, float radius, float h, int gridX, int gridY, float timestep = 0.015f);

    void updateFlip();
    const std::vector<glm::vec2>& getPos() { return m_partPos; };
    const std::vector<glm::vec2>& getVel() { return m_partVel; };
    std::vector<glm::vec4> getGrid(float width);
    std::vector<glm::vec4> getCells();
    std::vector<glm::vec4> getCellColors();
    void updateObstacle(glm::vec2 pos, glm::vec2 vel, float radius);
};

#endif