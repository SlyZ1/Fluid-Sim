#ifndef UI_UTILS_HPP
#define UI_UTILS_HPP

#include <string>
#include <imgui/imgui.h>

class UIUtils {
public:
    static inline void drawInput(const std::string& name, int& value) {
        ImGui::InputInt(("##" + name).c_str(), &value);
    }
    static inline void drawInput(const std::string& name, float& value) {
        ImGui::InputFloat(("##" + name).c_str(), &value);
    }

    static inline void drawDrag(const std::string& name, int& value) {
        ImGui::DragInt(("##" + name).c_str(), &value);
    }
    static inline void drawDrag(const std::string& name, float& value) {
        ImGui::DragFloat(("##" + name).c_str(), &value);
    }

    static inline void drawField(const std::string& name, int& value) {
        drawInput(name, value);
    }
    static inline void drawField(const std::string& name, float& value) {
        drawDrag(name, value);
    }
};

#endif