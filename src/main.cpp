#include <iostream>
#include <vector>
#include <string>

#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

#include "engine/solver_engine.hpp"

int main() {
    // 1. GLFW 初期化
    if (!glfwInit()) return -1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "QykIs2 - Rational Point Finder", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync 有効化

    // 2. ImGui & ImPlot 初期化
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // --------------------------------------------------
    // 探索エンジンと描画データの状態管理
    // --------------------------------------------------
    SolverEngine engine;

    // UI 設定入力値
    int selected_degree = 3; // 3, 4, 5
    int input_a = 0, input_b = 0, input_c = 0, input_d = 0;
    int max_d = 50;
    int max_X = 1000;

    // プロット用データキャッシュ (double 型)
    std::vector<double> plot_xs;
    std::vector<double> plot_ys;

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

        // --------------------------------------------------
        // 非同期スレッドから新しく発見された点を回収
        // --------------------------------------------------
        if (engine.is_running() || engine.current_d() > 0) {
            auto new_points = engine.pop_new_points();
            for (const auto& pt : new_points) {
                // 有理数 (int128_t) から double への変換
                double x_val = static_cast<double>(pt.num_x) / static_cast<double>(pt.den_x);
                double y_val = static_cast<double>(pt.num_y) / static_cast<double>(pt.den_y);

                plot_xs.push_back(x_val);
                plot_ys.push_back(y_val);

                // 文字列整形
                std::string x_str = std::to_string(static_cast<long long>(pt.num_x)) + "/" + std::to_string(static_cast<long long>(pt.den_x));
                std::string y_str = std::to_string(static_cast<long long>(pt.num_y)) + "/" + std::to_string(static_cast<long long>(pt.den_y));

                found_log.push_back({x_str, y_str, x_val, y_val});
            }
        }

        // ImGui フレーム開始
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // メイン画面全域ウィンドウ
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("QykIs2 Workspace", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        // --------------------------------------------------
        // 左ペイン: 設定 & ステータス
        // --------------------------------------------------
        ImGui::BeginChild("ControlPanel", ImVec2(340, 0), true);
        ImGui::Text("QykIs2 Engine Settings");
        ImGui::Separator();

        // 次数選択
        ImGui::Text("Curve Type:");
        ImGui::RadioButton("3rd (y^2 = x^3 + ax + b)", &selected_degree, 3);
        ImGui::RadioButton("4th (y^2 = x^4 + ax^2 + bx + c)", &selected_degree, 4);
        ImGui::RadioButton("5th (y^2 = x^5 + ax^3 + bx^2...)", &selected_degree, 5);

        ImGui::Spacing();
        ImGui::Text("Coefficients:");
        ImGui::InputInt("a", &input_a);
        ImGui::InputInt("b", &input_b);
        if (selected_degree >= 4) ImGui::InputInt("c", &input_c);
        if (selected_degree >= 5) ImGui::InputInt("d", &input_d);

        ImGui::Spacing();
        ImGui::Text("Search Limits:");
        ImGui::InputInt("Max d (Denom)", &max_d);
        ImGui::InputInt("Max X (Numer)", &max_X);

        ImGui::Spacing();
        ImGui::Separator();

        // スタート/ストップ ボタン制御
        if (!engine.is_running()) {
            if (ImGui::Button("Start Search", ImVec2(-1, 35))) {
                plot_xs.clear();
                plot_ys.clear();
                found_log.clear();

                CurveConfig config;
                if (selected_degree == 3) config.degree = CurveDegree::Degree3;
                else if (selected_degree == 4) config.degree = CurveDegree::Degree4;
                else if (selected_degree == 5) config.degree = CurveDegree::Degree5;

                config.a = input_a;
                config.b = input_b;
                config.c = input_c;
                config.d_coeff = input_d;

                engine.start_search(config, max_d, max_X);
            }
        } else {
            if (ImGui::Button("Stop Search", ImVec2(-1, 35))) {
                engine.stop_search();
            }
        }

        // ステータス表示
        ImGui::Spacing();
        ImGui::Text("Status: %s", engine.is_running() ? "SEARCHING..." : "IDLE");
        ImGui::Text("Current d: %lld / %d", static_cast<long long>(engine.current_d()), max_d);
        ImGui::Text("Total Checked: %llu", static_cast<unsigned long long>(engine.total_checked()));
        ImGui::Text("Points Found: %zu", found_log.size());

        ImGui::EndChild();

        ImGui::SameLine();

        // --------------------------------------------------
        // 右ペイン: グラフ描画 & 発見ログテーブル
        // --------------------------------------------------
        ImGui::BeginChild("RightPanel", ImVec2(0, 0), false);

        // 上半分の 60% を ImPlot グラフ表示
        float plot_height = ImGui::GetContentRegionAvail().y * 0.65f;
        if (ImPlot::BeginPlot("Rational Points Real-Plane", ImVec2(-1, plot_height))) {
            if (!plot_xs.empty()) {
                ImPlot::PlotScatter("Points", plot_xs.data(), plot_ys.data(), static_cast<int>(plot_xs.size()));
            }
            ImPlot::EndPlot();
        }

        // 下半分の 40% を発見された有理点リスト表示
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

        ImGui::EndChild();

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

    // 後処理
    engine.stop_search();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}