#ifndef SOLVER_GIZMOS_HPP
#define SOLVER_GIZMOS_HPP

#include <glm/glm.hpp>

#include "solvers/solverVisitor.hpp"
#include "gizmos/wireframes.hpp"
#include "core/camera.hpp"

class SolverGizmos : public ISolverVisitor {
private:
    std::unique_ptr<Wireframes> m_wireframes;
    glm::vec4 m_color = glm::vec4(1);

    glm::vec3 m_solverDomainSize = glm::vec3(0);

public:
    SolverGizmos(std::weak_ptr<Camera> camera);
    ~SolverGizmos() override;

    SolverGizmos(const SolverGizmos&) = delete;
    SolverGizmos& operator=(const SolverGizmos&) = delete;
    SolverGizmos(SolverGizmos&&) = default;
    SolverGizmos& operator=(SolverGizmos&&) = default;

    void visit(IParticleSolver& solver) override;
    void visit(FlipSolverCPU& solver) override;
    void visit(FlipSolverGPU& solver) override;

    void setColor(glm::vec4 color) { m_color = color; }
    void render() const;
};

#endif