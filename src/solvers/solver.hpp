#ifndef SOLVER_INTERFACE_HPP
#define SOLVER_INTERFACE_HPP

#include <memory>

struct ISolverConfig {
    virtual void drawImgui() const {}; 
};

class ISolver {
protected:
    std::unique_ptr<ISolverConfig> m_baseConfig;
public:
    ISolver(std::unique_ptr<ISolverConfig> config) : m_baseConfig(std::move(config)) {};
    virtual ~ISolver() = default;
    virtual void update() = 0;

    const ISolverConfig& getConfig() const {
        return *m_baseConfig;
    }
};

#endif