#include "ui/views.hpp"
#include "imgui.h"

void render_history_view(AppState& state) {
    ImGui::Text("Search History Log");
    ImGui::Separator();
    ImGui::Spacing();

    if (state.history.sessions.empty()) {
        ImGui::TextDisabled("No history available yet. Run a search first!");
    } else {
        if (ImGui::BeginTable("HistoryTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(-1, -100))) {
            ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("Degree", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("Range (d / X)", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Points Found", ImGuiTableColumnFlags_WidthFixed, 150.0f);
            ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 220.0f);
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < state.history.sessions.size(); ++i) {
                const auto& sess = state.history.sessions[i];
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0); ImGui::Text("#%zu", i + 1);
                ImGui::TableSetColumnIndex(1); ImGui::Text("%d-deg", sess.degree);
                ImGui::TableSetColumnIndex(2); ImGui::Text("d <= %d, X <= %d", sess.max_d, sess.max_X);
                ImGui::TableSetColumnIndex(3); ImGui::Text("%zu points", sess.points.size());

                ImGui::TableSetColumnIndex(4);
                ImGui::PushID(static_cast<int>(i));
                if (ImGui::Button("Load Params & Points")) {
                    state.history.current_index = static_cast<int>(i);
                    state.load_session(sess);
                    state.current_view = ViewMode::Workspace;
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Back to Workspace", ImVec2(220, 40))) {
        state.current_view = ViewMode::Workspace;
    }
}