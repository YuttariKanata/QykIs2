#include "ui/views.hpp"
#include "ui/ui_components.hpp"
#include "utils/string_utils.hpp"
#include "imgui.h"
#include <gmp.h>
#include <iostream>

// std::string を ImGui::InputText で扱うためのヘルパー
static bool InputTextString(const char* label, std::string& str, ImGuiInputTextFlags flags = 0) {
    char buf[128];
    snprintf(buf, sizeof(buf), "%s", str.c_str());
    if (ImGui::InputText(label, buf, sizeof(buf), flags)) {
        str = buf;
        return true;
    }
    return false;
}

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
            state.error_message.clear();

            // 1. 各パラメータの parse_rational_input チェック
            mpq_t mpq_a, mpq_b, mpq_c, mpq_d, mpq_e, mpq_f, mpq_g;
            mpq_inits(mpq_a, mpq_b, mpq_c, mpq_d, mpq_e, mpq_f, mpq_g, nullptr);

            bool ok = true;
            if (!parse_rational_input(mpq_a, state.input_a)) { state.error_message = "Invalid coefficient 'a'"; ok = false; }
            else if (!parse_rational_input(mpq_b, state.input_b)) { state.error_message = "Invalid coefficient 'b'"; ok = false; }
            else if (!parse_rational_input(mpq_c, state.input_c)) { state.error_message = "Invalid coefficient 'c'"; ok = false; }
            else if (!parse_rational_input(mpq_d, state.input_d)) { state.error_message = "Invalid coefficient 'd'"; ok = false; }
            else if (!parse_rational_input(mpq_e, state.input_e)) { state.error_message = "Invalid coefficient 'e'"; ok = false; }
            else if (state.selected_degree >= 4 && !parse_rational_input(mpq_f, state.input_f)) { state.error_message = "Invalid coefficient 'f'"; ok = false; }
            else if (state.selected_degree == 5 && !parse_rational_input(mpq_g, state.input_g)) { state.error_message = "Invalid coefficient 'g'"; ok = false; }

            // a = 0 や b = 0 (最高次数が潰れる) のチェック
            if (ok && mpq_sgn(mpq_a) == 0) {
                state.error_message = "Coefficient 'a' cannot be zero";
                ok = false;
            }
            if (ok && mpq_sgn(mpq_b) == 0) {
                state.error_message = "Highest degree coefficient 'b' cannot be zero";
                ok = false;
            }

            if (!ok) {
                mpq_clears(mpq_a, mpq_b, mpq_c, mpq_d, mpq_e, mpq_f, mpq_g, nullptr);
            } else {
                state.found_log.clear();

                // --- [DEBUG LOG] パース結果の標準エラー出力 ---
                std::cerr << "\n========== [DEBUG: Search Initiated] ==========\n";
                std::cerr << "Degree: " << state.selected_degree << "\n";

                // mpq_t を文字列化して出力するヘルパー（または gmp_fprintf を使用）
                auto print_mpq = [](const char* name, const mpq_t q) {
                    char* str = mpq_get_str(nullptr, 10, q);
                    std::cerr << "  Coeff " << name << " = " << str << "\n";
                    void (*freefunc)(void *, size_t);
                    mp_get_memory_functions(nullptr, nullptr, &freefunc);
                    freefunc(str, strlen(str) + 1); // GMPが確保したメモリの解放
                };

                print_mpq("a", mpq_a);
                print_mpq("b", mpq_b);
                print_mpq("c", mpq_c);
                print_mpq("d", mpq_d);
                print_mpq("e", mpq_e);
                if (state.selected_degree >= 4) print_mpq("f", mpq_f);
                if (state.selected_degree == 5) print_mpq("g", mpq_g);

                std::cerr << "Limits: max_d = " << state.max_d << ", max_X = " << state.max_X << "\n";
                std::cerr << "===============================================\n" << std::endl;

                // 2. 将来的にここで一般形 -> 標準形（Weierstrass等）の有理数変形を実施
                // 一旦暫定の int 変換でエンジンに渡す（後ほど有理数対応に拡張）
                CurveConfig config;
                if (state.selected_degree == 3) config.degree = CurveDegree::Degree3;
                else if (state.selected_degree == 4) config.degree = CurveDegree::Degree4;
                else if (state.selected_degree == 5) config.degree = CurveDegree::Degree5;

                config.a = static_cast<int>(mpz_get_si(mpq_numref(mpq_c)));
                config.b = static_cast<int>(mpz_get_si(mpq_numref(mpq_d)));
                config.c = static_cast<int>(mpz_get_si(mpq_numref(mpq_e)));
                config.d_coeff = static_cast<int>(mpz_get_si(mpq_numref(mpq_f)));

                mpq_clears(mpq_a, mpq_b, mpq_c, mpq_d, mpq_e, mpq_f, mpq_g, nullptr);

                state.engine.start_search(config, state.max_d, state.max_X);
            }
        }
    }

    ImGui::Spacing();
    ImGui::Text("Status: %s", state.engine.is_running() ? "SEARCHING..." : "IDLE");
    ImGui::Text("Current d: %lld / %d", static_cast<int64_t>(state.engine.current_d()), state.max_d);
    ImGui::Text("Total Checked: %llu", static_cast<uint64_t>(state.engine.total_checked()));
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