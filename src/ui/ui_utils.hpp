#ifndef UI_UTILS_HPP
#define UI_UTILS_HPP

#include <string>
#include <imgui/imgui.h>
#include <algorithm>

class UIUtils {
public:
    static inline void drawInput(const std::string& name, int& value, int min, int max) {
        value = std::clamp(value, min, max);
        ImGui::InputInt(("##" + name).c_str(), &value);
        value = std::clamp(value, min, max);
    }
    static inline void drawInput(const std::string& name, float& value, float min, float max) {
        value = std::clamp(value, min, max);
        ImGui::InputFloat(("##" + name).c_str(), &value);
        value = std::clamp(value, min, max);
    }

    static inline void drawDrag(const std::string& name, int& value, int min, int max) {
        ImGui::DragInt(("##" + name).c_str(), &value, 1, min, max);
    }
    static inline void drawDrag(const std::string& name, float& value, float min, float max) {
        ImGui::DragFloat(("##" + name).c_str(), &value, 0.01f, min, max);
    }

    static inline void drawField(const std::string& name, int& value, int min, int max) {
        drawInput(name, value, min, max);
    }
    static inline void drawField(const std::string& name, float& value, float min, float max) {
        drawDrag(name, value, min, max);
    }
};

#endif