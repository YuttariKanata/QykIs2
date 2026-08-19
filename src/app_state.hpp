#ifndef QYKIS2_APP_STATE_HPP
#define QYKIS2_APP_STATE_HPP

#include "engine/solver_engine.hpp"
#include "utils/history_manager.hpp"

enum class ViewMode {
    Workspace,
    History,
    Settings
};

struct AppState {
    ViewMode current_view = ViewMode::Workspace;
    
    // 見栄え設定
    float font_scale = 2.0f;
    int theme_style = 0;
    float left_pane_width = 450.0f;

    // 探索パラメータ入力値
    int selected_degree = 3;
    int input_a = 1, input_b = 1, input_c = 0, input_d = 0, input_e = 0;
    int input_f = 0, input_g = 0;
    int max_d = 50;
    int max_X = 1000;

    // ログ＆バックエンド
    std::vector<PointLog> found_log;
    SolverEngine engine;
    HistoryManager history;

    bool was_running_last_frame = false;

    void load_session(const SearchSession& sess) {
        selected_degree = sess.degree;
        input_a = sess.a;
        input_b = sess.b;
        input_c = sess.c;
        input_d = sess.d;
        input_e = sess.e;
        input_f = sess.f;
        input_g = sess.g;
        max_d = sess.max_d;
        max_X = sess.max_X;
        found_log = sess.points;
    }
};

#endif