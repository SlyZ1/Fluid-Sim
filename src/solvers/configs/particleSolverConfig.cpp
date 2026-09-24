#include "particleSolverConfig.hpp"

#include "../../ui/ui_utils.hpp"
#include "../../ui/ui.hpp"

using namespace std;

void IParticleSolverConfig::drawImgui(){
    UI::Label("partN");
    UIUtils::drawField("partN", m_partN, 1, (int)1e8);
    UI::Label("partRadius");
    UIUtils::drawField("partRadius", m_partRadius, 1, 200);
    UI::Label("dt");
    UIUtils::drawField("dt", m_dt, 0.05f, 0.1f);
}

unique_ptr<ISolverConfig> IParticleSolverConfig::clone() const {
    return make_unique<IParticleSolverConfig>(*this);
}