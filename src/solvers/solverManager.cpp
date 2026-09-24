#include "solverManager.hpp"

using namespace std;

SolverManager::SolverManager() {}

SolverManager::~SolverManager() {}

weak_ptr<FlipSolverCPU> SolverManager::instantiate(const FlipSolverCPUConfig& config) {
    shared_ptr<FlipSolverCPU> solver = make_shared<FlipSolverCPU>(config);
    m_solver = solver;
    return solver;
}

weak_ptr<FlipSolverGPU> SolverManager::instantiate(const FlipSolverGPUConfig& config) {
    shared_ptr<FlipSolverGPU> solver = make_shared<FlipSolverGPU>(config);
    m_solver = solver;
    return solver;
}