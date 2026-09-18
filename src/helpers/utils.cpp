#include "utils.hpp"
#include <sstream>
#include <iomanip>

using namespace glm;
using namespace std;

string Utils::toString(const glm::vec2& v, int precision) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "(%.*f, %.*f)", precision, v.x, precision, v.y);
    return string(buffer);
}

string Utils::toString(const glm::vec3& v, int precision) {
    char buffer[96];
    snprintf(buffer, sizeof(buffer), "(%.*f, %.*f, %.*f)", precision, v.x, precision, v.y, precision, v.z);
    return string(buffer);
}

string Utils::toString(const glm::vec4& v, int precision) {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "(%.*f, %.*f, %.*f, %.*f)", precision, v.x, precision, v.y, precision, v.z, precision, v.w);
    return string(buffer);
}

string Utils::formatFloat(float f, int precision, bool scientific){
    ostringstream oss;
    auto prefix = scientific ? std::scientific : fixed;
    oss << prefix << std::setprecision(precision) << f;
    return oss.str();
}

