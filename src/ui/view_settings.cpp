#include "ui/views.hpp"
#include "imgui.h"

void render_settings_view(AppState& state, void(*apply_theme)(int)) {
    ImGui::Text("Global Application Settings");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::SliderFloat("Font Scale", &state.font_scale, 1.0f, 3.0f, "%.1f");

    ImGui::Text("Theme:");
    if (ImGui::RadioButton("Dark", &state.theme_style, 0)) apply_theme(0);
    ImGui::SameLine();
    if (ImGui::RadioButton("Light", &state.theme_style, 1)) apply_theme(1);
    ImGui::SameLine();
    if (ImGui::RadioButton("Classic", &state.theme_style, 2)) apply_theme(2);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Back to Workspace", ImVec2(330, 40))) {
        state.current_view = ViewMode::Workspace;
    }
}