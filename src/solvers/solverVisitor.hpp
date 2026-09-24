#ifndef SOLVER_VISITOR_HPP
#define SOLVER_VISITOR_HPP

class IParticleSolver;
class FlipSolverCPU;
class FlipSolverGPU;

class ISolverVisitor {
public:
    virtual ~ISolverVisitor() = default;

    virtual void visit(IParticleSolver& solver) const = 0;
    virtual void visit(FlipSolverCPU& solver) const = 0;
    virtual void visit(FlipSolverGPU& solver) const = 0;
};

#endif