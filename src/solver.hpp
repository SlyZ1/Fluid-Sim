#ifndef SOLVER_HPP
#define SOLVER_HPP

#include <vector>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include <iostream>
#include "helpers/utils.hpp"

using namespace std;
using namespace glm;

class Solver{
private:
    int partN = 0;
    vector<vec2> partPos = {};
    vector<vec2> partVel = {};

    vec2 obstaclePos = vec2(0.f);
    vec2 obstacleVel = vec2(0.f);
    float obstacleRadius = 0.f;

    float radius = 0.f;
    float h = 0.f;
    int gridX = 0;
    int gridY = 0;
    vector<float> oldVelX = {};
    vector<float> oldVelY = {};
    vector<float> velX = {};
    vector<float> velY = {};
    vector<float> rX = {};
    vector<float> rY = {};
    vector<bool> isAir = {};
    vector<bool> isWall = {};

    vector<vector<int>> particlesInGrid = {};

    float dt = 0.f;

    ivec2 cellToCoord(int cell, int nx);
    int coordToCell(ivec2 coord, int nx, int ny);
    int posToCell(vec2 pos, int nx, int ny);
    vec2 cellToPos(int cell, int nx, int ny);
    vec2 coordToPos(ivec2 coord, int nx, int ny);

    void posToCoordAndDp(vec2 pos, int nx, int ny, ivec2& coord, vec2& dp);

    void simulateParticles();
    void pushAppartParticles();
    void particleCollisions();
    void particlesToGrid();
    void solveIncompressibility(int iterations);
    void gridToParticles();

public:
    Solver(int partN, float radius, float h, int gridX, int gridY, float timestep = 0.015f);

    void updateFlip();
    const vector<vec2>& getPos() { return partPos; };
    const vector<vec2>& getVel() { return partVel; };
    vector<vec4> getGrid(float width);
    vector<vec4> getCells();
    vector<vec4> getCellColors();
    void updateObstacle(vec2 pos, vec2 vel, float radius);
};

#endif