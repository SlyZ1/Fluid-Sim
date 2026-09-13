#ifndef UI_HPP
#define UI_HPP

#include "../app.hpp"
#include "../helpers/stats.hpp"
#include "ui_colors.hpp"

struct UIContext {
    shared_ptr<App> app;
};

class UI {
    private:
    UIContext m_ctx = {};
    vector<shared_ptr<Stats>> m_statsCtx = {};

    void TextWithShadow(
        const char* text,
        ImVec4 textColor = ImVec4(1,1,1,1),
        ImVec4 shadowColor = ImVec4(0.05f,0.05f,0.05f,0.7f),
        ImVec2 offset = ImVec2(1,1)
    ) const;
    void renderToolTip(const string& tip) const;

    void Label(
        const char* label, 
        const string& desc = "", 
        function<void(void)> customWidget = nullptr, 
        float widgetSize = 0.0f
    ) const;

    void BeginTwoColumnLayout() const;
    void EndTwoColumnLayout() const;

    void renderStats();

    public:
    UI(UIContext ctx) : m_ctx(std::move(ctx)) {};
    void setStatsContext(const vector<shared_ptr<IStatsProvider>>& ctx);

    void render();
};

#endif