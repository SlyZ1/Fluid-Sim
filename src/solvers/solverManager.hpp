#ifndef SOLVER_MANAGER_HPP
#define SOLVER_MANAGER_HPP

#include "flipSolverCPU.hpp"
#include "flipSolverGPU.hpp"

class SolverManager {
private:
    std::shared_ptr<ISolver> m_solver;
public:
    SolverManager();
    ~SolverManager();

    SolverManager(const SolverManager&) = delete;
    SolverManager& operator=(const SolverManager&) = delete;

    std::weak_ptr<FlipSolverCPU> instantiate(const FlipSolverCPUConfig& config);
    std::weak_ptr<FlipSolverGPU> instantiate(const FlipSolverGPUConfig& config);
};

#endif