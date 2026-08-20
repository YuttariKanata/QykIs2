#include "ui/views.hpp"
#include "ui/ui_components.hpp"
#include "utils/string_utils.hpp"
#include "math/curve_transform.hpp"
#include "imgui.h"
#include <gmp.h>
#include <gmpxx.h>
#include <iostream>


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
    ImGui::Text("Coefficients (Integers or Fractions like 1/3):");
    
    // 入力変更時にエラー表示をクリア
    if (InputTextString("a (y^2 coeff)", state.input_a)) state.error_message.clear();
    if (InputTextString("b (highest)", state.input_b)) state.error_message.clear();
    if (InputTextString("c", state.input_c)) state.error_message.clear();
    if (InputTextString("d", state.input_d)) state.error_message.clear();
    if (InputTextString("e", state.input_e)) state.error_message.clear();

    if (state.selected_degree >= 4) {
        if (InputTextString("f", state.input_f)) state.error_message.clear();
    }
    if (state.selected_degree == 5) {
        if (InputTextString("g", state.input_g)) state.error_message.clear();
    }

    ImGui::Spacing();
    ImGui::Text("Search Limits:");
    ImGui::InputInt("Max d (Denom)", &state.max_d);
    ImGui::InputInt("Max X (Numer)", &state.max_X);

    ImGui::Spacing();

    // 入力エラーメッセージの描画
    if (!state.error_message.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "[Error] %s", state.error_message.c_str());
        ImGui::Spacing();
    }

    ImGui::Separator();

    // スタート / ストップ ボタン
if (state.engine.is_searching()) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));

        if (ImGui::Button("Stop Search", ImVec2(-1, 40))) {
            state.engine.stop_search();
        }

        ImGui::PopStyleColor(3);
    } else {
        if (ImGui::Button("Start Search", ImVec2(-1, 40))) {
            state.error_message.clear();

            mpq_class q_a, q_b, q_c, q_d, q_e, q_f, q_g;

            bool ok = true;
            if (!parse_rational_input(q_a, state.input_a))      { state.error_message = "Invalid coefficient 'a'"; ok = false; }
            else if (!parse_rational_input(q_b, state.input_b)) { state.error_message = "Invalid coefficient 'b'"; ok = false; }
            else if (!parse_rational_input(q_c, state.input_c)) { state.error_message = "Invalid coefficient 'c'"; ok = false; }
            else if (!parse_rational_input(q_d, state.input_d)) { state.error_message = "Invalid coefficient 'd'"; ok = false; }
            else if (!parse_rational_input(q_e, state.input_e)) { state.error_message = "Invalid coefficient 'e'"; ok = false; }
            else if (state.selected_degree >= 4 && !parse_rational_input(q_f, state.input_f)) { state.error_message = "Invalid coefficient 'f'"; ok = false; }
            else if (state.selected_degree == 5 && !parse_rational_input(q_g, state.input_g)) { state.error_message = "Invalid coefficient 'g'"; ok = false; }

            if (ok && q_a == 0) {
                state.error_message = "Coefficient 'a' cannot be zero";
                ok = false;
            }
            if (ok && q_b == 0) {
                state.error_message = "Highest degree coefficient 'b' cannot be zero";
                ok = false;
            }

            if (ok) {
                state.found_log.clear();

                StandardCurveConfig std_config;
                CurveTransformInfo transform_info;

                bool norm_ok = normalize_curve(
                    state.selected_degree,
                    q_a, q_b, q_c, q_d, q_e, q_f, q_g,
                    std_config, transform_info
                );

                if (!norm_ok) {
                    state.error_message = "Curve coefficients exceed 128-bit limit after normalization.";
                } else {
                    // 逆変換用情報を保持
                    state.active_transform = transform_info;

                    // 探索開始
                    state.engine.start_search(std_config, transform_info, state.max_d, state.max_X);
                }
            }
        }
    }

    ImGui::Spacing();
    ImGui::Text("Status: %s", state.engine.is_searching() ? "SEARCHING..." : "IDLE");
    ImGui::Text("Progress: %.1f%%", state.engine.get_progress() * 100.0);
    ImGui::Text("Points Found: %zu", state.found_log.size());

    ImGui::Spacing();
    ImGui::Separator();
    if (ImGui::Button("History Log", ImVec2(-1, 35))) {
        state.current_view = ViewMode::History;
    }
    if (ImGui::Button("Settings", ImVec2(-1, 35))) {
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

    // --------------------------------------------------
    // SolverEngine から未処理の解 (u, v) を回収して逆変換
    // (テーブル描画の直前で log を最新化する)
    // --------------------------------------------------
    auto raw_points = state.engine.pop_found_points();
    for (const auto& pt : raw_points) {
        mpq_class orig_x, orig_y;
        if (map_point_to_original(pt.u, pt.v, pt.d, state.selected_degree, state.active_transform, orig_x, orig_y)) {
            std::string x_str = orig_x.get_str(); // "3/4" のような分数文字列ができる
            std::string y_str = orig_y.get_str();
            double x_val = orig_x.get_d();
            double y_val = orig_y.get_d();

            state.found_log.push_back({x_str, y_str, x_val, y_val});
        }
    }

    // テーブル表示
    ImGui::Text("Found Rational Points List:");
    if (ImGui::BeginTable("PointsTable", 3, 
            ImGuiTableFlags_Borders     |
            ImGuiTableFlags_RowBg       |
            ImGuiTableFlags_ScrollY     |
            ImGuiTableFlags_Resizable   |
            ImGuiTableFlags_Reorderable |
            ImGuiTableFlags_Hideable    ,
            ImVec2(-1, -1))) {
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