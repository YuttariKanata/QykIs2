#ifndef QYKIS2_APP_STATE_HPP
#define QYKIS2_APP_STATE_HPP

#include <string>
#include <vector>
#include "math/curve_transform.hpp"
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

    // 探索パラメータ入力値（有理数文字列で保持）
    int selected_degree = 3;
    std::string input_a = "1";
    std::string input_b = "1";
    std::string input_c = "0";
    std::string input_d = "0";
    std::string input_e = "0";
    std::string input_f = "0";
    std::string input_g = "0";
    int max_d = 50;
    int max_X = 1000;

    // 逆変換用情報をフレーム間で保持する変数
    CurveTransformInfo active_transform;

    // エラーハンドリング用
    std::string error_message = "";

    // ログ＆バックエンド
    std::vector<PointLog> found_log;
    SolverEngine engine;
    HistoryManager history;

    bool was_running_last_frame = false;

    // 履歴ロード（SearchSession 側も string 化に合わせて更新）
    void load_session(const SearchSession& sess) {
        selected_degree = sess.degree;
        input_a = sess.a_str;
        input_b = sess.b_str;
        input_c = sess.c_str;
        input_d = sess.d_str;
        input_e = sess.e_str;
        input_f = sess.f_str;
        input_g = sess.g_str;
        max_d = sess.max_d;
        max_X = sess.max_X;
        found_log = sess.points;
        error_message.clear();
    }
};

#endif // QYKIS2_APP_STATE_HPP