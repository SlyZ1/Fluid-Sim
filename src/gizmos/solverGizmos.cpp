#include "solverGizmos.hpp"

#include "solvers/flipSolverCPU.hpp"
#include "solvers/flipSolverGPU.hpp"

using namespace std;
using namespace glm;

SolverGizmos::SolverGizmos(weak_ptr<Camera> camera) {
    m_wireframes = make_unique<Wireframes>(camera);
}

SolverGizmos::~SolverGizmos() {}

void SolverGizmos::visit(IParticleSolver&) {}

void SolverGizmos::visit(FlipSolverCPU& solver) {
    const FlipSolverCPUConfig& config = solver.getDraftConfig();
    
    vec3 newDomainSize = vec3(config.getDomainSize(), 0);
    if (newDomainSize != m_solverDomainSize){
        m_wireframes->reset();
        m_solverDomainSize = newDomainSize;
        vec3 max = m_solverDomainSize * 0.5f;
        m_wireframes->addBox2D(-max, max, m_color);
        m_wireframes->uploadData();
    }
}

void SolverGizmos::visit(FlipSolverGPU& solver) {
    const FlipSolverGPUConfig& config = solver.getDraftConfig();
    
    vec3 newDomainSize = config.getDomainSize();
    if (newDomainSize != m_solverDomainSize){
        m_wireframes->reset();
        m_solverDomainSize = newDomainSize;
        vec3 max = config.getDomainSize() * 0.5f;
        m_wireframes->addBox(-max, max, m_color);
        m_wireframes->uploadData();
    }
}

void SolverGizmos::render() const {
    m_wireframes->render();
}