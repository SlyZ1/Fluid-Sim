#ifndef FLIP_SOLVER_GPU_CONFIG
#define FLIP_SOLVER_GPU_CONFIG

#include "particleSolverConfig.hpp"

#include <glm/glm.hpp>

class FlipSolverGPUConfig : public IParticleSolverConfig {
protected:
    float m_partPerH = 2;
    glm::vec3 m_domainSize = glm::vec3(1.0f);

public:
    FlipSolverGPUConfig() : IParticleSolverConfig() {}
    FlipSolverGPUConfig(int partN, float partRadius, float dt, float partPerH, glm::vec3 domainSize) 
    : IParticleSolverConfig(partN, partRadius, dt), m_partPerH(partPerH), m_domainSize(domainSize) {}
    ~FlipSolverGPUConfig() override = default;

    std::unique_ptr<ISolverConfig> clone() const override; 

    float getH() const { return 2 * m_partRadius * m_partPerH; }
    float getDensity() const { return m_partPerH * m_partPerH * m_partPerH; }
    
    float getPartPerH() const { return m_partPerH; }
    void setPartPerH(float partPerH) { m_partPerH = std::max(partPerH, 1.0f); }

    glm::vec3 getDomainSize() const { return m_domainSize; }
    void setDomainSize(glm::vec3 domainSize) { m_domainSize = glm::max(domainSize, 0.f); }


    glm::ivec3 getGridDim() const {
        float h = getH();
        return glm::ivec3(
            glm::max(1, (int)glm::floor(m_domainSize.x / h)),
            glm::max(1, (int)glm::floor(m_domainSize.y / h)),
            glm::max(1, (int)glm::floor(m_domainSize.z / h))
        );
    }
    int getGridX() const { return getGridDim().x; } 
    int getGridY() const { return getGridDim().y; }
    int getGridZ() const { return getGridDim().z; }
};

#endif