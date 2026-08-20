#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "app_state.hpp"
#include "ui/views.hpp"
#include "utils/string_utils.hpp"

static void apply_theme(int style) {
    if (style == 0) ImGui::StyleColorsDark();
    else if (style == 1) ImGui::StyleColorsLight();
    else if (style == 2) ImGui::StyleColorsClassic();
}

int main() {
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
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    AppState state;
    apply_theme(state.theme_style);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // 1. バックエンドから解を回収
        if (state.engine.is_running() || state.engine.current_d() > 0) {
            auto new_points = state.engine.pop_new_points();
            for (const auto& pt : new_points) {
                double x_val = static_cast<double>(pt.num_x) / static_cast<double>(pt.den_x);
                double y_val = static_cast<double>(pt.num_y) / static_cast<double>(pt.den_y);

                std::string x_str = to_string_128(pt.num_x) + "/" + to_string_128(pt.den_x);
                std::string y_str = to_string_128(pt.num_y) + "/" + to_string_128(pt.den_y);

                state.found_log.push_back({x_str, y_str, x_val, y_val});
            }
        }

        // 2. 探索完了検知 & 履歴追加
        if (state.was_running_last_frame && !state.engine.is_running()) {
            SearchSession sess;
            sess.degree = state.selected_degree;
            sess.a_str = state.input_a;
            sess.b_str = state.input_b;
            sess.c_str = state.input_c;
            sess.d_str = state.input_d;
            sess.e_str = state.input_e;
            sess.f_str = state.input_f;
            sess.g_str = state.input_g;
            sess.max_d = state.max_d;
            sess.max_X = state.max_X;
            sess.points = state.found_log;

            state.history.push_session(sess);
        }
        state.was_running_last_frame = state.engine.is_running();

        // 3. UI 描画
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        io.FontGlobalScale = state.font_scale;

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("QykIs2 Workspace", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        switch (state.current_view) {
            case ViewMode::Workspace:
                render_workspace_view(state);
                break;
            case ViewMode::History:
                render_history_view(state);
                break;
            case ViewMode::Settings:
                render_settings_view(state, apply_theme);
                break;
        }

        ImGui::End();

        // 4. レンダリング
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    state.engine.stop_search();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}