#ifndef SOLVER_UI_HPP
#define SOLVER_UI_HPP

#include "solvers/solverVisitor.hpp"

class SolverUI : public ISolverVisitor {
public:
    ~SolverUI() override = default;

    void visit(IParticleSolver& solver) override;
    void visit(FlipSolverCPU& solver) override;
    void visit(FlipSolverGPU& solver) override;
};

#endif