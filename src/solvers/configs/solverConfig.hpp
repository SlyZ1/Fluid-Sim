#ifndef SOLVER_CONFIG_HPP 
#define SOLVER_CONFIG_HPP 

#include <memory>

class ISolverConfig {
public:
    ISolverConfig() {};
    virtual ~ISolverConfig() = default;
    virtual std::unique_ptr<ISolverConfig> clone() const = 0;
};

#endif