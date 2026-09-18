#ifndef UI_HPP
#define UI_HPP

#include "../app.hpp"
#include "../helpers/stats.hpp"

struct UIContext {
    std::shared_ptr<App> app;
};

class UI {
private:
    UIContext m_ctx = {};
    std::vector<std::shared_ptr<Stats>> m_statsCtx = {};

    void TextWithShadow(
        const char* text,
        ImVec4 textColor = ImVec4(1,1,1,1),
        ImVec4 shadowColor = ImVec4(0.05f,0.05f,0.05f,0.7f),
        ImVec2 offset = ImVec2(1,1)
    ) const;
    void renderToolTip(const std::string& tip) const;

    void Label(
        const char* label, 
        const std::string& desc = "", 
        std::function<void(void)> customWidget = nullptr, 
        float widgetSize = 0.0f
    ) const;

    void AlignInputToRight(const char* input) const;

    void BeginTwoColumnLayout(float columnRatio = 0.4f) const;
    void EndTwoColumnLayout() const;

    void drawTimers(StatsPrimitive<float> timers);
    void drawCounters(StatsPrimitive<int> counters);
    void drawStorages(StatsPrimitive<int> storage);
    void renderStats();

public:
    UI(UIContext ctx) : m_ctx(std::move(ctx)) {};
    void setStatsContext(const std::vector<std::shared_ptr<IStatsProvider>>& ctx);

    void render();
};

#endif