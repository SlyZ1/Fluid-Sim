#ifndef RENDERER_TEMPLATE_HPP
#define RENDERER_TEMPLATE_HPP

#include "../solvers/solver.hpp"
#include <memory>

class IRenderer {
public:
    IRenderer() {}
    virtual ~IRenderer() = default;
    virtual void render() = 0;
    virtual void reload() = 0;
};

#endif