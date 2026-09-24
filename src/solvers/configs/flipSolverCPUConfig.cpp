#include "flipSolverCPUConfig.hpp"

#include "../../ui/ui_utils.hpp"
#include "../../ui/ui.hpp"

using namespace std;

void FlipSolverCPUConfig::drawImgui() {
    IParticleSolverConfig::drawImgui();

    UI::Label("hPartRatio");
    UIUtils::drawField("hPartRatio", m_hPartRatio, 1, 10);
    UI::Label("gridX");
    UIUtils::drawField("gridX", m_gridX, 1, 10000);
    UI::Label("gridY");
    UIUtils::drawField("gridY", m_gridY, 1, 10000);
}

unique_ptr<ISolverConfig> FlipSolverCPUConfig::clone() const {
    return make_unique<FlipSolverCPUConfig>(*this);
}