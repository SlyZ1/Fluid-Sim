#include "flipSolverGPUConfig.hpp"

#include "../../ui/ui_utils.hpp"
#include "../../ui/ui.hpp"

using namespace std;

void FlipSolverGPUConfig::drawImgui() {
    IParticleSolverConfig::drawImgui();

    UI::Label("hPartRatio");
    UIUtils::drawField("hPartRatio", m_hPartRatio, 1, 10);
    UI::Label("gridX");
    UIUtils::drawField("gridX", m_gridX, 1, 10000);
    UI::Label("gridY");
    UIUtils::drawField("gridY", m_gridY, 1, 10000);
    UI::Label("gridZ");
    UIUtils::drawField("gridZ", m_gridZ, 1, 10000);
}

unique_ptr<ISolverConfig> FlipSolverGPUConfig::clone() const {
    return make_unique<FlipSolverGPUConfig>(*this);
}