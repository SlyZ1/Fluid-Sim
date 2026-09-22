#ifndef SOLVER_INTERFACE_HPP
#define SOLVER_INTERFACE_HPP

#include <memory>
#include <glad/glad.h>

#include "../helpers/stats.hpp"

struct ISolverConfig {
    virtual void drawImgui() const = 0; 
};

#define PARTICLE_CONFIG_FIELDS(X) \
    X(int, partN, 0) \
    X(float, partRadius, 0) \
    X(float, dt, 0.015f) 

struct IParticleSolverConfig : public ISolverConfig {
#define DECLARE_FIELDS(type, name, val) type name = val;
    PARTICLE_CONFIG_FIELDS(DECLARE_FIELDS)
#undef DECLARE_FIELDS
};

class ISolver : public IStatsProvider {
protected:
    std::unique_ptr<ISolverConfig> m_baseConfig;
public:
    ISolver(std::unique_ptr<ISolverConfig> config, const std::string& statsName) : IStatsProvider(statsName), m_baseConfig(std::move(config)) {};
    virtual ~ISolver() = default;
    virtual void update() = 0;
    virtual void reload() = 0;
    virtual GLuint getPosBuffer() const = 0;
    virtual GLuint getVelBuffer() const = 0;

    virtual const ISolverConfig& getConfig() const {
        return *m_baseConfig;
    }
};

class IParticleSolver : public ISolver {
public:
    IParticleSolver(std::unique_ptr<IParticleSolverConfig> config, const std::string& statsName) : ISolver(std::move(config), statsName) {};
    virtual ~IParticleSolver() = default;

    virtual const IParticleSolverConfig& getConfig() const override {
        return static_cast<const IParticleSolverConfig&>(*m_baseConfig); 
    }
};

#endif