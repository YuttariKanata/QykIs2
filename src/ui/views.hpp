#ifndef QYKIS2_UI_VIEWS_HPP
#define QYKIS2_UI_VIEWS_HPP

#include "../app_state.hpp"

void render_workspace_view(AppState& state);
void render_history_view(AppState& state);
void render_settings_view(AppState& state, void(*apply_theme)(int));

#endif