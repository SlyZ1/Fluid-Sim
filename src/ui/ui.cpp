#include "ui.hpp" 
#include "ui_colors.hpp"
#include <format>

#include "../solvers/flipSolverGPU.hpp"

using namespace std;

void UI::setStatsContext(const vector<shared_ptr<IStatsProvider>>& ctx){
    m_statsCtx = vector<shared_ptr<Stats>>(ctx.size());
    for (int i = 0; i < (int)ctx.size(); i++)
        m_statsCtx[i] = ctx[i]->getStats(); 
}


// ----------------------------------------- USEFUL FUNCS -----------------------------------------

void UI::TextWithShadow(const char* text, ImVec4 textColor, ImVec4 shadowColor, ImVec2 offset) {
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

void UI::renderToolTip(const string& tip) {
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(300.0f);
        ImGui::TextUnformatted(tip.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void UI::Label(const char* label, const string& desc, function<void(void)> customWidget, float widgetSize)
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

void UI::AlignInputToRight(const char* input) {
    auto posX = (ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(input).x 
    - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
    if(posX > ImGui::GetCursorPosX())
        ImGui::SetCursorPosX(posX);
}

void UI::BeginTwoColumnLayout(float columnRatio)
{
    float availWidth = ImGui::GetContentRegionAvail().x;
    float labelWidth = std::max(availWidth * columnRatio, 120.0f);
    ImGui::BeginTable("##layout", 2, ImGuiTableFlags_SizingStretchProp);
    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelWidth);
    ImGui::TableSetupColumn("Input", ImGuiTableColumnFlags_WidthStretch);
}

void UI::EndTwoColumnLayout()
{
    ImGui::EndTable();
}

bool UI::BeginCustomHeader(const string& name) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, UIColors::fgColor);
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.0f);
    ImGui::BeginChild((name + "_group").c_str(), ImVec2(-FLT_MIN, 0), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysUseWindowPadding);
    bool open = ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth);
    if (open){
        ImGui::Spacing();
        ImGui::Spacing();
    }
    return open;
}

void UI::EndCustomHeader() {
    ImGui::EndChild();
    ImGui::PopStyleVar(1);
    ImGui::PopStyleColor(1);
}


// ----------------------------------------- STATS -----------------------------------------

void UI::drawTimers(StatsPrimitive<float> timers){
    Stat<TimerValue> totalTimeStat = timers.get(0);
    if (timers.numStats() <= 2){
        BeginTwoColumnLayout();
        Label(totalTimeStat.label.c_str());
        string text = Utils::formatFloat(totalTimeStat.value, 2) + "ms";
        AlignInputToRight(text.c_str());
        ImGui::Text("%s", text.c_str());
        EndTwoColumnLayout();
    }
    else {
        ImGui::Indent();

        string headerName = "Latencies: " + Utils::formatFloat(totalTimeStat.value, 2) + "ms###latencies";
        if (!ImGui::CollapsingHeader(headerName.c_str())) { ImGui::Unindent(); return; }

        BeginTwoColumnLayout();
        vector<StatIndex> timerPermutation = timers.getSortPermutation();
        float totalStatTime = timers.get(timerPermutation[0]).value;
        for (const StatIndex& i : timerPermutation)
        {
            if (i == 0) continue;
            Stat<TimerValue> stat = timers.get(i);
            Label(stat.label.c_str());
            string percentage = " (" + to_string(glm::clamp((int)(100 * stat.value / totalStatTime), 0, 100)) + "%)";
            if (i == timerPermutation[0]) percentage = "";
            string text = Utils::formatFloat(stat.value, 2) + "ms" + percentage;
            AlignInputToRight(text.c_str());
            ImGui::Text("%s", text.c_str());
        }
        EndTwoColumnLayout();
        ImGui::Unindent();
    }
}

void UI::drawCounters(StatsPrimitive<int> counters){
    if (counters.numStats() > 1){
        ImGui::Indent();
        if (!ImGui::CollapsingHeader("Counters")) { ImGui::Unindent(); return; }
    }

    BeginTwoColumnLayout();
    for (StatIndex i = 0; i < counters.numStats(); i++)
    {
        Stat<CounterValue> stat = counters.get(i);
        Label(stat.label.c_str());
        string text = stat.value >= 10000 ? Utils::formatFloat(stat.value, 2, true) : to_string(stat.value);
        AlignInputToRight(text.c_str());
        ImGui::Text("%s", text.c_str());
    }
    EndTwoColumnLayout();

    if (counters.numStats() > 1) ImGui::Unindent();
}

void UI::drawStorages(StatsPrimitive<int> storages){
    if (storages.numStats() > 1){
        ImGui::Indent();
        if (!ImGui::CollapsingHeader("Storages")) { ImGui::Unindent(); return; }
    }

    BeginTwoColumnLayout();
    for (StatIndex i = 0; i < storages.numStats(); i++)
    {
        Stat<StorageValue> stat = storages.get(i);
        Label(stat.label.c_str());
        string suffix = Metrics::storageSuffix(stat.value);
        string text = to_string(stat.value) + suffix;
        AlignInputToRight(text.c_str());
        ImGui::Text("%s", text.c_str());
    }
    EndTwoColumnLayout();

    if (storages.numStats() > 1) ImGui::Unindent();
}

void UI::renderStats(){
    ImGuiIO& io = ImGui::GetIO();
    ImGuiWindowFlags flags =  ImGuiWindowFlags_AlwaysAutoResize
                            | ImGuiWindowFlags_NoCollapse
                            | ImGuiWindowFlags_NoDecoration
                            | ImGuiWindowFlags_NoMove
                            | ImGuiWindowFlags_NoResize;
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x, ImGui::GetMainViewport()->WorkPos.y), ImGuiCond_Always, ImVec2(1, 0));
    ImGui::SetNextWindowSize(ImVec2(270.f, 0.f));
    if (ImGui::Begin("Stats", nullptr, flags)) {
        for (const shared_ptr<Stats>& stats : m_statsCtx)
        {
            if (ImGui::CollapsingHeader(stats->name.c_str())) {
                drawTimers(stats->getTimers());
                drawCounters(stats->getCounters());
                drawStorages(stats->getStorages());
            }
            if (stats->numStats() > 0) ImGui::Dummy(ImVec2(0, 2.f));
        }
    }
    ImGui::End();
}


// ----------------------------------------- PARAMS -----------------------------------------


void UI::renderParams(){
    float padding = ImGui::GetStyle().WindowPadding.y;

    ImGui::BeginChild("Parameters", ImVec2(-FLT_MIN, -FLT_MIN), ImGuiChildFlags_Borders);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 6.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, UIColors::lightBlueBorder);
    ImGui::BeginChild("Scrollable Parameters", ImVec2(-FLT_MIN, -padding), ImGuiChildFlags_None, ImGuiWindowFlags_AlwaysVerticalScrollbar);

    if (BeginCustomHeader("Simulation")){
        
        ImGui::TreePop();
    } EndCustomHeader();

    if (BeginCustomHeader("Solver")){
        BeginTwoColumnLayout();
        ImGui::BeginDisabled();
        m_ctx.solver->getConfig().drawImgui();
        ImGui::EndDisabled();
        EndTwoColumnLayout();
        ImGui::TreePop();
    } EndCustomHeader();

    if (BeginCustomHeader("Renderer")){
        BeginTwoColumnLayout();

        EndTwoColumnLayout();
        ImGui::TreePop();
    } EndCustomHeader();

    ImGui::EndChild();
    ImGui::PopStyleColor(1);
    ImGui::PopStyleVar(2);
    ImGui::EndChild();
}


// ----------------------------------------- MAIN -----------------------------------------

void UI::render(){ 
    float viewPortVerticalPos = ImGui::GetMainViewport()->WorkPos.y;
    float viewPortVerticalSize = ImGui::GetMainViewport()->WorkSize.y;

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

        ImGui::PushStyleColor(ImGuiCol_WindowBg, UIColors::bgColor);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, UIColors::mgColor);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.1f, 0.1f, 0.1f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.1f, 0.1f, 0.1f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));
        
            ImGui::SetNextWindowPos(ImVec2(0, viewPortVerticalPos), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(300, viewPortVerticalSize), ImGuiCond_Once);
            ImGui::SetNextWindowSizeConstraints(ImVec2(300, viewPortVerticalSize), 
                                                ImVec2(ImGui::GetIO().DisplaySize.x / 2.0f, viewPortVerticalSize));
            ImGui::Begin("Left Window", (bool*)NULL, ImGuiWindowFlags_NoCollapse 
                                                    | ImGuiWindowFlags_NoTitleBar 
                                                    | ImGuiWindowFlags_NoBringToFrontOnFocus);
                                                    
            // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            // renderScene();
            // ImGui::PopStyleVar(1);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
            renderParams();
            ImGui::PopStyleVar(1);

            ImGui::End();
        
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(4);

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(4);
}