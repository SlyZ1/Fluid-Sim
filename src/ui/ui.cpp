#include "ui.hpp" 

void UI::setStatsContext(const vector<shared_ptr<IStatsProvider>>& ctx){
    m_statsCtx = vector<shared_ptr<Stats>>(ctx.size());
    for (int i = 0; i < (int)ctx.size(); i++)
        m_statsCtx[i] = ctx[i]->getStats();
}

void UI::TextWithShadow(const char* text, ImVec4 textColor, ImVec4 shadowColor, ImVec2 offset) const {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float rowHeight = ImGui::GetTextLineHeightWithSpacing();
    float textHeight = ImGui::GetTextLineHeight();

    pos.y += (rowHeight - textHeight) * 0.5f;
    ImDrawList* draw = ImGui::GetWindowDrawList();
    
    draw->AddText(ImVec2(pos.x + offset.x, pos.y + offset.y), ImGui::GetColorU32(shadowColor), text);
    draw->AddText(pos, ImGui::GetColorU32(textColor), text);
    
    ImVec2 textSize = ImGui::CalcTextSize(text);
    ImGui::Dummy(textSize);
}

void UI::renderToolTip(const string& tip) const {
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(300.0f);
        ImGui::TextUnformatted(tip.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void UI::Label(const char* label, const string& desc, function<void(void)> customWidget, float widgetSize) const
{
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    TextWithShadow(label, ImVec4(1,1,1,1), ImVec4(0.05f,0.05f,0.05f,0.7f), ImVec2(1,1));
    if (!desc.empty())
        renderToolTip(desc); 
    if (customWidget != nullptr){
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - widgetSize);
        customWidget();
    }
    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-FLT_MIN); 
}

void UI::BeginTwoColumnLayout() const
{
    float availWidth = ImGui::GetContentRegionAvail().x;
    float labelWidth = std::max(availWidth * 0.4f, 120.0f);
    ImGui::BeginTable("##layout", 2, ImGuiTableFlags_SizingStretchProp);
    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelWidth);
    ImGui::TableSetupColumn("Input", ImGuiTableColumnFlags_WidthStretch);
}

void UI::EndTwoColumnLayout() const
{
    ImGui::EndTable();
}

void UI::renderStats(){
    ImGuiIO& io = ImGui::GetIO();
    ImGuiWindowFlags flags =  ImGuiWindowFlags_AlwaysAutoResize
                            | ImGuiWindowFlags_NoCollapse
                            | ImGuiWindowFlags_NoDecoration
                            | ImGuiWindowFlags_NoMove
                            | ImGuiWindowFlags_NoMouseInputs
                            | ImGuiWindowFlags_NoResize;
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x, ImGui::GetMainViewport()->WorkPos.y), ImGuiCond_Always, ImVec2(1, 0));
    if (ImGui::Begin("Stats", nullptr, flags)) {
        for (const shared_ptr<Stats>& stats : m_statsCtx)
        {
            //TextWithShadow(stats->name.c_str(), ImVec4(1,1,1,1), ImVec4(1,1,1,1));
            ImGui::SeparatorText(stats->name.c_str());
            
            BeginTwoColumnLayout();
            for (int i = 0; i < stats->numStats(); i++)
            {
                Stat stat = stats->get(i);
                Label(stat.label.c_str());
                ImGui::Text("%.2f", stat.time);
            }
            EndTwoColumnLayout();
        }
    }
    ImGui::End();
}

void UI::render(){ 
    ImGui::PushStyleColor(ImGuiCol_Header, UIColors::mgColor);
    ImGui::PushStyleColor(ImGuiCol_TitleBg, UIColors::mgColor);
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, UIColors::mgColor);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, UIColors::mgColor);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 0.0f);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, UIColors::fgColor);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);

        renderStats();

        ImGui::PopStyleColor(1);
        ImGui::PopStyleVar(1);

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(4);
}