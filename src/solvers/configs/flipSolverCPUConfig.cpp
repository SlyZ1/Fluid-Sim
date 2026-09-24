#include "flipSolverCPUConfig.hpp"

using namespace std;

unique_ptr<ISolverConfig> FlipSolverCPUConfig::clone() const {
    return make_unique<FlipSolverCPUConfig>(*this);
}