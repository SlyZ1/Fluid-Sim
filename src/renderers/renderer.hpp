#ifndef RENDERER_TEMPLATE_HPP
#define RENDERER_TEMPLATE_HPP

#include "../solvers/solver.hpp"

class IRenderer {
protected:
    const ISolver& m_solver;
public:
    IRenderer(const ISolver& solver) : m_solver(solver) {};
    virtual ~IRenderer() = default;
    virtual void render() = 0;
    virtual void reload() = 0;
};

#endif