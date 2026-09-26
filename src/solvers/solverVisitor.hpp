#ifndef SOLVER_VISITOR_HPP
#define SOLVER_VISITOR_HPP

class IParticleSolver;
class FlipSolverCPU;
class FlipSolverGPU;

class ISolverVisitor {
public:
    virtual ~ISolverVisitor() = default;

    virtual void visit(IParticleSolver& solver) = 0;
    virtual void visit(FlipSolverCPU& solver) = 0;
    virtual void visit(FlipSolverGPU& solver) = 0;
};

#endif