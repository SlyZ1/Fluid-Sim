#include "solverUI.hpp"

#include "utilsUI.hpp"
#include "solvers/flipSolverCPU.hpp"
#include "solvers/flipSolverGPU.hpp"

using namespace glm;

#define DRAW_FIELD(drawingFunction, label, type, name, uppercasedName, min, max) \
    UtilsUI::Label(label); \
    type name = config.get##uppercasedName(); \
    drawingFunction(#name, name, min, max); \
    config.set##uppercasedName(name);


void SolverUI::visit(IParticleSolver& solver) {
    IParticleSolverConfig& config = static_cast<IParticleSolverConfig&>(solver.getDraftConfig());

    UtilsUI::BeginTwoColumnLayout();

    DRAW_FIELD(UtilsUI::drawInput, "Particle Number", int, partN, PartN, 0, (int)1e8)
    DRAW_FIELD(UtilsUI::drawDrag, "Particle Radius", float, partRadius, PartRadius, 1, 200)
    DRAW_FIELD(UtilsUI::drawDrag, "Timestep", float, dt, Dt, 0.01f, 0.1f)

    UtilsUI::EndTwoColumnLayout();
}

void SolverUI::visit(FlipSolverCPU& solver) {
    visit(static_cast<IParticleSolver&>(solver));
    FlipSolverCPUConfig& config = static_cast<FlipSolverCPUConfig&>(solver.getDraftConfig());

    UtilsUI::BeginTwoColumnLayout();

    DRAW_FIELD(UtilsUI::drawDrag, "Particle Per Cell Size", float, partPerH, PartPerH, 1, 10)
    DRAW_FIELD(UtilsUI::drawDrag, "Overrelaxation", float, overrelaxation, Overrelaxation, 1, 2)
    DRAW_FIELD(UtilsUI::drawDrag, "Domain Size", vec2, domainSize, DomainSize, 0, 10000)

    UtilsUI::EndTwoColumnLayout();
}

void SolverUI::visit(FlipSolverGPU& solver) {
    visit(static_cast<IParticleSolver&>(solver));
    FlipSolverGPUConfig& config = static_cast<FlipSolverGPUConfig&>(solver.getDraftConfig());

    UtilsUI::BeginTwoColumnLayout();

    DRAW_FIELD(UtilsUI::drawDrag, "Particle Per Cell Size", float, partPerH, PartPerH, 1, 10)
    DRAW_FIELD(UtilsUI::drawDrag, "Domain Size", vec3, domainSize, DomainSize, 0, 10000)

    UtilsUI::EndTwoColumnLayout();
}
