#ifndef FLIP_SOLVER_CPU_CONFIG
#define FLIP_SOLVER_CPU_CONFIG
 
#include "particleSolverConfig.hpp"

#include <glm/glm.hpp>

class FlipSolverCPUConfig : public IParticleSolverConfig {
protected:
    float m_hPartRatio = 2;
    int m_gridX = 1;
    int m_gridY = 1;
    float m_overrelaxation = 1.9f;

public:
    FlipSolverCPUConfig() : IParticleSolverConfig() {}
    FlipSolverCPUConfig(int partN, float partRadius, float dt, float hPartRatio, int overrelaxation) 
    : IParticleSolverConfig(partN, partRadius, dt), m_hPartRatio(hPartRatio), m_overrelaxation(overrelaxation) {}
    ~FlipSolverCPUConfig() override = default;

    void drawImgui() override; 
    std::unique_ptr<ISolverConfig> clone() const override; 

    constexpr float getH() const {
        return 2 * m_partRadius * m_hPartRatio;
    }

    constexpr float getDensity() const {
        return m_hPartRatio * m_hPartRatio;
    }

    void setDimensions(glm::vec2 dim){
        m_gridX = floor(dim.x / getH());
        m_gridY = floor(dim.y / getH());
    }

    int getGridX() const { return m_gridX; } 
    int getGridY() const { return m_gridY; } 
    float getOverrelaxation() const { return m_overrelaxation; }
};

#endif