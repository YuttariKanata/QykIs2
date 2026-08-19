#include "ui/views.hpp"
#include "ui/ui_components.hpp"
#include "imgui.h"

void render_workspace_view(AppState& state) {
    // 1. 左ペイン (ControlPanel)
    ImGui::BeginChild("ControlPanel", ImVec2(state.left_pane_width, 0), true);

    // 履歴ナビゲーション [<] [>]
    ImGui::BeginDisabled(!state.history.can_undo());
    if (ImGui::Button("<", ImVec2(40, 30))) {
        state.history.undo();
        if (auto* sess = state.history.current_session()) {
            state.load_session(*sess);
        }
    }
    ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::BeginDisabled(!state.history.can_redo());
    if (ImGui::Button(">", ImVec2(40, 30))) {
        state.history.redo();
        if (auto* sess = state.history.current_session()) {
            state.load_session(*sess);
        }
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    if (state.history.current_index >= 0) {
        ImGui::Text("History [%d/%d]", state.history.current_index + 1, static_cast<int>(state.history.sessions.size()));
    } else {
        ImGui::Text("History [No Logs]");
    }

    ImGui::Separator();

    ImGui::Text("QykIs2 Engine Settings");
    ImGui::Separator();

    ImGui::Text("Curve Type:");
    ImGui::RadioButton("3rd (ay^2 = bx^3 + cx^2 + dx + e)", &state.selected_degree, 3);
    ImGui::RadioButton("4th (ay^2 = bx^4 + cx^3 + dx^2 + ex + f)", &state.selected_degree, 4);
    ImGui::RadioButton("5th (ay^2 = bx^5 + cx^4 + dx^3 + ex^2 + fx + g)", &state.selected_degree, 5);

    ImGui::Spacing();
    ImGui::Text("Coefficients:");
    ImGui::InputInt("a (y^2 coeff)", &state.input_a);
    ImGui::InputInt("b (highest)", &state.input_b);
    ImGui::InputInt("c", &state.input_c);
    ImGui::InputInt("d", &state.input_d);
    ImGui::InputInt("e", &state.input_e);

    if (state.selected_degree >= 4) {
        ImGui::InputInt("f", &state.input_f);
    }
    if (state.selected_degree == 5) {
        ImGui::InputInt("g", &state.input_g);
    }

    ImGui::Spacing();
    ImGui::Text("Search Limits:");
    ImGui::InputInt("Max d (Denom)", &state.max_d);
    ImGui::InputInt("Max X (Numer)", &state.max_X);

    ImGui::Spacing();
    ImGui::Separator();

    // スタート / ストップ ボタン
    if (state.engine.is_running()) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));

        if (ImGui::Button("Stop Search", ImVec2(-1, 40))) {
            state.engine.stop_search();
        }

        ImGui::PopStyleColor(3);
    } else {
        if (ImGui::Button("Start Search", ImVec2(-1, 40))) {
            state.found_log.clear();

            CurveConfig config;
            if (state.selected_degree == 3) config.degree = CurveDegree::Degree3;
            else if (state.selected_degree == 4) config.degree = CurveDegree::Degree4;
            else if (state.selected_degree == 5) config.degree = CurveDegree::Degree5;

            config.a = state.input_c;
            config.b = state.input_d;
            config.c = state.input_e;
            config.d_coeff = state.input_f;

            state.engine.start_search(config, state.max_d, state.max_X);
        }
    }

    ImGui::Spacing();
    ImGui::Text("Status: %s", state.engine.is_running() ? "SEARCHING..." : "IDLE");
    ImGui::Text("Current d: %lld / %d", static_cast<int64_t>(state.engine.current_d()), state.max_d);
    ImGui::Text("Total Checked: %llu", static_cast<uint64_t>(state.engine.total_checked()));
    ImGui::Text("Points Found: %zu", state.found_log.size());

    ImGui::Spacing();
    ImGui::Separator();
    if (ImGui::Button("History Log", ImVec2(150, 35))) {
        state.current_view = ViewMode::History;
    }
    ImGui::SameLine();
    if (ImGui::Button("Settings", ImVec2(150, 35))) {
        state.current_view = ViewMode::Settings;
    }

    ImGui::EndChild();

    ImGui::SameLine();

    // 2. QSplitter
    float splitter_thickness = 8.0f;
    Splitter("##splitter", &splitter_thickness, &state.left_pane_width, 250.0f, 300.0f);

    ImGui::SameLine();

    // 3. 右ペイン (Table View)
    ImGui::BeginChild("RightPanel", ImVec2(0, 0), true);

    ImGui::Text("Found Rational Points List:");
    if (ImGui::BeginTable("PointsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(-1, -1))) {
        ImGui::TableSetupColumn("x = num/den", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("y = num/den", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Approx (x, y)", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        for (const auto& log : state.found_log) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("%s", log.x_str.c_str());
            ImGui::TableSetColumnIndex(1); ImGui::Text("%s", log.y_str.c_str());
            ImGui::TableSetColumnIndex(2); ImGui::Text("(%.4f, %.4f)", log.x_val, log.y_val);
        }
        ImGui::EndTable();
    }

    ImGui::EndChild();
}