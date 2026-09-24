#include "utils.hpp"
#include <sstream>
#include <iomanip>
#include <filesystem>

using namespace glm;
using namespace std;
namespace fs = std::filesystem;

string Utils::toString(const vec2& v, int precision) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "(%.*f, %.*f)", precision, v.x, precision, v.y);
    return string(buffer);
}

string Utils::toString(const vec3& v, int precision) {
    char buffer[96];
    snprintf(buffer, sizeof(buffer), "(%.*f, %.*f, %.*f)", precision, v.x, precision, v.y, precision, v.z);
    return string(buffer);
}

string Utils::toString(const vec4& v, int precision) {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "(%.*f, %.*f, %.*f, %.*f)", precision, v.x, precision, v.y, precision, v.z, precision, v.w);
    return string(buffer);
}

string Utils::formatFloat(float f, int precision, bool scientific){
    ostringstream oss;
    auto prefix = scientific ? std::scientific : fixed;
    oss << prefix << setprecision(precision) << f;
    return oss.str();
}

string Utils::joinPath(const string& p1, const string& p2){
    fs::path path;
    path += p1;
    path += p2;
    return path.string();
}

vec2 Utils::xy(const vec3& v){
    return vec2(v.x, v.y);
}

vec2 Utils::xy(const vec4& v){
    return vec2(v.x, v.y);
}

vec2 Utils::xz(const vec3& v){
    return vec2(v.x, v.z);
}

vec2 Utils::xz(const vec4& v){
    return vec2(v.x, v.z);
}

vec2 Utils::yz(const vec3& v){
    return vec2(v.y, v.z);
}

vec2 Utils::yz(const vec4& v){
    return vec2(v.y, v.z);
}

vec3 Utils::xyz(const vec4& v){
    return vec3(v.x, v.y, v.z);
}
