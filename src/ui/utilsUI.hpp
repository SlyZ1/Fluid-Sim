#ifndef UTILS_UI_HPP
#define UTILS_UI_HPP

#include <string>
#include <imgui/imgui.h>
#include <algorithm>
#include <functional>
#include <glm/glm.hpp>

class UtilsUI {
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
    static inline void drawInput(const std::string& name, glm::vec2& value, float min, float max) {
        value = glm::clamp(value, min, max);
        ImGui::InputFloat2(("##" + name).c_str(), &value.x);
        value = glm::clamp(value, min, max);
    }
    static inline void drawInput(const std::string& name, glm::vec3& value, float min, float max) {
        value = glm::clamp(value, min, max);
        ImGui::InputFloat3(("##" + name).c_str(), &value.x);
        value = glm::clamp(value, min, max);
    }

    static inline void drawDrag(const std::string& name, int& value, int min, int max) {
        ImGui::DragInt(("##" + name).c_str(), &value, 1, min, max);
    }
    static inline void drawDrag(const std::string& name, float& value, float min, float max) {
        ImGui::DragFloat(("##" + name).c_str(), &value, 0.01f, min, max);
    }
    static inline void drawDrag(const std::string& name, glm::vec2& value, float min, float max) {
        ImGui::DragFloat2(("##" + name).c_str(), &value.x, 0.5f, min, max);
    }
    static inline void drawDrag(const std::string& name, glm::vec3& value, float min, float max) {
        ImGui::DragFloat3(("##" + name).c_str(), &value.x, 0.5f, min, max);
    }

    static void TextWithShadow(
        const char* text,
        ImVec4 textColor = ImVec4(1,1,1,1),
        ImVec4 shadowColor = ImVec4(0.05f,0.05f,0.05f,0.7f),
        ImVec2 offset = ImVec2(1,1)
    );
    static void renderToolTip(const std::string& tip);

    static void AlignInputToRight(const char* input);

    static bool BeginCustomHeader(const std::string& name);
    static void EndCustomHeader();

    static void BeginTwoColumnLayout(float columnRatio = 0.4f);
    static void EndTwoColumnLayout();
    static void Label(
        const char* label, 
        const std::string& desc = "", 
        std::function<void(void)> customWidget = nullptr, 
        float widgetSize = 0.0f
    );
};

#endif