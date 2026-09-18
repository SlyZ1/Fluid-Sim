#ifndef UTILS_HPP
#define UTILS_HPP

#include <glm/glm.hpp>
#include <string>
#include <vector>

class Utils {
public:
    static std::string toString(const glm::vec2& v, int precision = 3);
    static std::string toString(const glm::vec3& v, int precision = 3);
    static std::string toString(const glm::vec4& v, int precision = 3);
    static std::string formatFloat(float f, int precision = 2, bool scientific = false);
    
    template <typename T>
    static std::vector<T> concat(std::initializer_list<std::reference_wrapper<const std::vector<T>>> vectors){
        std::vector<T> result;
        for (const auto& vRef : vectors) {
            const std::vector<T>& v = vRef.get();
            result.insert(result.end(), v.begin(), v.end());
        }
        return result;
    }
};

#endif