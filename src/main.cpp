#include <iostream>
#include <vector>

#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

int main() {
    // 1. GLFW 初期化
    if (!glfwInit()) return -1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(1024, 600, "QykIs2 - Rational Point Finder", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // 2. ImGui & ImPlot 初期化
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // ダークモード適用
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // テスト用のサンプルデータ (楕円曲線上の点のイメージ)
    std::vector<double> xs = { 3.0, 1.29, 1.21 };
    std::vector<double> ys = { 5.0, 0.383, 0.28 };

    // 3. メインUIループ
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 画面全体のウィンドウ
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("QykIs2 Main", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

        // 左ペイン: 設定パネル
        ImGui::BeginChild("ControlPanel", ImVec2(300, 0), true);
        ImGui::Text("QykIs2 Engine Settings");
        ImGui::Separator();
        
        static int degree = 3;
        ImGui::RadioButton("3rd Degree (y^2 = x^3...)", &degree, 3);
        ImGui::RadioButton("4th Degree (y^2 = x^4...)", &degree, 4);

        if (ImGui::Button("Start Search", ImVec2(-1, 30))) {
            std::cout << "Search Started!" << std::endl;
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // 右ペイン: グラフ描画
        ImGui::BeginChild("PlotPanel", ImVec2(0, 0), true);
        if (ImPlot::BeginPlot("Rational Points Real-Plane", ImVec2(-1, -1))) {
            ImPlot::PlotScatter("Points", xs.data(), ys.data(), xs.size());
            ImPlot::EndPlot();
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

    // クリーンアップ
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}