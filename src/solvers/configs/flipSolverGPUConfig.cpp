#include "flipSolverGPUConfig.hpp"

using namespace std;

unique_ptr<ISolverConfig> FlipSolverGPUConfig::clone() const {
    return make_unique<FlipSolverGPUConfig>(*this);
}