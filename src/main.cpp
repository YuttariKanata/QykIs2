#include <iostream>
#include <vector>
#include <string>

#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "engine/solver_engine.hpp"

// ImGui で QSplitter 風の挙動を実現する関数
inline void Splitter(const char* name, float* thickness, float* left_width, float min_left, float min_right) {
    ImGui::PushID(name);
    
    // 見た目の割れ目（縦棒）を描画
    ImGui::Button("##splitter_btn", ImVec2(*thickness, -1.0f));

    // ホバー時にカーソルを左右拡大アイコンに変更
    if (ImGui::IsItemHovered()) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
    }

    // ドラッグ操作によるペイン幅の更新
    if (ImGui::IsItemActive()) {
        float mouse_delta = ImGui::GetIO().MouseDelta.x;
        if (mouse_delta != 0.0f) {
            *left_width += mouse_delta;
            float max_width = ImGui::GetContentRegionAvail().x + *left_width - min_right;
            if (*left_width < min_left) *left_width = min_left;
            if (*left_width > max_width) *left_width = max_width;
        }
    }

    ImGui::PopID();
}

int main() {
    // 1. GLFW 初期化
    if (!glfwInit()) return -1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(1280, 800, "QykIs2 - Rational Point Finder", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync 有効化

    // 2. ImGui 初期化
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    // 高DPI対応: フォント表示サイズを倍にして読みやすくする
    io.FontGlobalScale = 2.0f;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // --------------------------------------------------
    // UI テーマ・フォント設定
    // --------------------------------------------------
    float font_scale = 2.0f; // フォント倍率 (1.0f ~ 3.0f)
    int theme_style = 0;     // 0: Dark, 1: Light, 2: Classic

    auto apply_theme = [](int style) {
        if (style == 0) ImGui::StyleColorsDark();
        else if (style == 1) ImGui::StyleColorsLight();
        else if (style == 2) ImGui::StyleColorsClassic();
    };

    // 初期テーマの適用
    apply_theme(theme_style);

    // UI 画面切替 & Splitter 状態
    bool show_settings_view = false; // true なら設定画面を表示
    float left_pane_width = 450.0f;  // 左ペインの幅 (QSplitter 風に動かせる)

    // --------------------------------------------------
    // 状態管理
    // --------------------------------------------------
    SolverEngine engine;

    // ユーザー入力 (一般形: ay^2 = bx^3 + cx^2 + dx + e)
    int selected_degree = 3;
    int input_a = 1, input_b = 1, input_c = 0, input_d = 0, input_e = 0;
    int max_d = 50;
    int max_X = 1000;

    // テーブル表示用ログ構造体
    struct PointLog {
        std::string x_str;
        std::string y_str;
        double x_val;
        double y_val;
    };
    std::vector<PointLog> found_log;

    // 3. メイン UI ループ
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // バックエンドから新解を回収
        if (engine.is_running() || engine.current_d() > 0) {
            auto new_points = engine.pop_new_points();
            for (const auto& pt : new_points) {
                double x_val = static_cast<double>(pt.num_x) / static_cast<double>(pt.den_x);
                double y_val = static_cast<double>(pt.num_y) / static_cast<double>(pt.den_y);

                std::string x_str = std::to_string(static_cast<long long>(pt.num_x)) + "/" + std::to_string(static_cast<long long>(pt.den_x));
                std::string y_str = std::to_string(static_cast<long long>(pt.num_y)) + "/" + std::to_string(static_cast<long long>(pt.den_y));

                found_log.push_back({x_str, y_str, x_val, y_val});
            }
        }

        // ImGui フレーム開始
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // フォントスケールの動的更新
        io.FontGlobalScale = font_scale;

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("QykIs2 Workspace", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        // --- メイン画面を描画するか、設定全画面を描画するかの分岐 ---
        if (show_settings_view) {
            // --------------------------------------------------
            // 全画面: Settings View
            // --------------------------------------------------
            ImGui::Text("Global Application Settings");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::SliderFloat("Font Scale", &font_scale, 1.0f, 3.0f, "%.1f");

            ImGui::Text("Theme:");
            if (ImGui::RadioButton("Dark", &theme_style, 0)) apply_theme(0);
            ImGui::SameLine();
            if (ImGui::RadioButton("Light", &theme_style, 1)) apply_theme(1);
            ImGui::SameLine();
            if (ImGui::RadioButton("Classic", &theme_style, 2)) apply_theme(2);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // 元のワークスペースへ戻るボタン
            if (ImGui::Button("Back to Workspace", ImVec2(200, 40))) {
                show_settings_view = false;
            }

        } else {
            // --------------------------------------------------
            // メイン画面: Workspace (QSplitter 風 2 ペイン)
            // --------------------------------------------------
            
            // 1. 左ペイン (ControlPanel)
            ImGui::BeginChild("ControlPanel", ImVec2(left_pane_width, 0), true);
            
            // 上部: エンジン設定 & コントロール
            ImGui::Text("QykIs2 Engine Settings");
            ImGui::Separator();

            // 次数選択ラジオボタン
            ImGui::Text("Curve Type:");
            ImGui::RadioButton("3rd (ay^2 = bx^3 + cx^2 + dx + e)", &selected_degree, 3);
            ImGui::RadioButton("4th (ay^2 = bx^4 + cx^3 + dx^2 + ex + f)", &selected_degree, 4);
            ImGui::RadioButton("5th (ay^2 = bx^5 + cx^4 + dx^3 + ex^2 + fx + g)", &selected_degree, 5);

            ImGui::Spacing();
            ImGui::Text("Coefficients:");
            ImGui::InputInt("a (y^2 coeff)", &input_a);
            ImGui::InputInt("b (highest)", &input_b);
            ImGui::InputInt("c", &input_c);
            ImGui::InputInt("d", &input_d);
            ImGui::InputInt("e", &input_e);

            // 4次以上で f を表示
            static int input_f = 0;
            if (selected_degree >= 4) {
                ImGui::InputInt("f", &input_f);
            }

            // 5次で g を表示
            static int input_g = 0;
            if (selected_degree == 5) {
                ImGui::InputInt("g", &input_g);
            }

            ImGui::Spacing();
            ImGui::Text("Search Limits:");
            ImGui::InputInt("Max d (Denom)", &max_d);
            ImGui::InputInt("Max X (Numer)", &max_X);

            ImGui::Spacing();
            ImGui::Separator();

            // --- ControlPanel 内のボタン描画部分 ---

            if (engine.is_running()) {
                // 探索中は赤っぽい色の Stop ボタンを表示
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));

                if (ImGui::Button("Stop Search", ImVec2(-1, 40))) {
                    engine.stop_search();
                }

                ImGui::PopStyleColor(3);
            } else {
                // 停止中・完了時は Start ボタンを表示
                if (ImGui::Button("Start Search", ImVec2(-1, 40))) {
                    found_log.clear();

                    CurveConfig config;
                    if (selected_degree == 3) config.degree = CurveDegree::Degree3;
                    else if (selected_degree == 4) config.degree = CurveDegree::Degree4;
                    else if (selected_degree == 5) config.degree = CurveDegree::Degree5;

                    config.a = input_c;
                    config.b = input_d;
                    config.c = input_e;
                    config.d_coeff = input_f;

                    engine.start_search(config, max_d, max_X);
                }
            }

            ImGui::Spacing();
            ImGui::Text("Status: %s", engine.is_running() ? "SEARCHING..." : "IDLE");
            ImGui::Text("Current d: %lld / %d", static_cast<long long>(engine.current_d()), max_d);
            ImGui::Text("Total Checked: %llu", static_cast<unsigned long long>(engine.total_checked()));
            ImGui::Text("Points Found: %zu", found_log.size());

            // 左下に配置する Settings ボタン
            // (ペインの下部に押し出すようにダミーを挟むか、最下部に配置)
            ImGui::Spacing();
            ImGui::Separator();
            if (ImGui::Button("Settings", ImVec2(-1, 35))) {
                show_settings_view = true;
            }

            ImGui::EndChild(); // ControlPanel 終了

            ImGui::SameLine();

            // 2. QSplitter ハンドル（境界線をドラッグ可能に）
            float splitter_thickness = 8.0f;
            Splitter("##splitter", &splitter_thickness, &left_pane_width, 250.0f, 300.0f);

            ImGui::SameLine();

            // 3. 右ペイン (Table View)
            ImGui::BeginChild("RightPanel", ImVec2(0, 0), true);
            
            ImGui::Text("Found Rational Points List:");
            if (ImGui::BeginTable("PointsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(-1, -1))) {
                ImGui::TableSetupColumn("x = num/den", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("y = num/den", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Approx (x, y)", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                for (const auto& log : found_log) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::Text("%s", log.x_str.c_str());
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%s", log.y_str.c_str());
                    ImGui::TableSetColumnIndex(2); ImGui::Text("(%.4f, %.4f)", log.x_val, log.y_val);
                }
                ImGui::EndTable();
            }

            ImGui::EndChild(); // RightPanel 終了
        }


        ImGui::End();

        // レンダリング
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // クリーンアップ
    engine.stop_search();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}