#ifndef UTILS_HPP
#define UTILS_HPP

#include <glm/glm.hpp>
#include <string>
#include <vector>

using namespace glm;
using namespace std;

class Utils {
public:
    static string toString(const vec2& v, int precision = 3);
    static string toString(const vec3& v, int precision = 3);
    static string toString(const vec4& v, int precision = 3);
    static string formatFloat(float f, int precision = 2, bool scientific = false);
    
    template <typename T>
    static vector<T> concat(initializer_list<reference_wrapper<const vector<T>>> vectors){
        vector<T> result;
        for (const auto& vRef : vectors) {
            const vector<T>& v = vRef.get();
            result.insert(result.end(), v.begin(), v.end());
        }
        return result;
    }
};

#endif