#ifndef QYKIS2_UI_UI_COMPONENTS_HPP
#define QYKIS2_UI_UI_COMPONENTS_HPP

#include "imgui.h"

inline void Splitter(const char* name, float* thickness, float* left_width, float min_left, float min_right) {
    ImGui::PushID(name);
    ImGui::Button("##splitter_btn", ImVec2(*thickness, -1.0f));

    if (ImGui::IsItemHovered()) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
    }

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

#endif // QYKIS2_UI_UI_COMPONENTS_HPP