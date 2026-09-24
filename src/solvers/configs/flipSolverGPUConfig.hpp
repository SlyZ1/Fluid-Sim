#ifndef FLIP_SOLVER_GPU_CONFIG
#define FLIP_SOLVER_GPU_CONFIG

#include "particleSolverConfig.hpp"

#include <glm/glm.hpp>

class FlipSolverGPUConfig : public IParticleSolverConfig {
protected:
    float m_hPartRatio = 2;
    int m_gridX = 1;
    int m_gridY = 1;
    int m_gridZ = 1;

public:
    FlipSolverGPUConfig() : IParticleSolverConfig() {}
    FlipSolverGPUConfig(int partN, float partRadius, float dt, float hPartRatio) 
    : IParticleSolverConfig(partN, partRadius, dt), m_hPartRatio(hPartRatio) {}
    ~FlipSolverGPUConfig() override = default;

    void drawImgui() override; 
    std::unique_ptr<ISolverConfig> clone() const override; 

    constexpr float getH() const {
        return 2 * m_partRadius * m_hPartRatio;
    }

    constexpr float getDensity() const {
        return m_hPartRatio * m_hPartRatio * m_hPartRatio;
    }

    void setDimensions(glm::vec3 dim){
        m_gridX = floor(dim.x / getH());
        m_gridY = floor(dim.y / getH());
        m_gridZ = floor(dim.z / getH());
    }

    int getGridX() const { return m_gridX; } 
    int getGridY() const { return m_gridY; }
    int getGridZ() const { return m_gridZ; }
};

#endif