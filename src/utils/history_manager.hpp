#ifndef QYKIS2_UTILS_HISTORY_MANAGER_HPP
#define QYKIS2_UTILS_HISTORY_MANAGER_HPP

#include <string>
#include <vector>

// テーブル表示用ログ構造体
struct PointLog {
    std::string x_str;
    std::string y_str;
    double x_val;
    double y_val;
};

// 1回の探索の全状態（パラメータ＋結果）を保持する構造体
struct SearchSession {
    int degree = 3;
    std::string a_str = "1";
    std::string b_str = "1";
    std::string c_str = "0";
    std::string d_str = "0";
    std::string e_str = "0";
    std::string f_str = "0";
    std::string g_str = "0";
    int max_d = 50;
    int max_X = 1000;
    std::string formula_str;
    std::vector<PointLog> points;
};

// 履歴スタック管理クラス
class HistoryManager {
public:
    std::vector<SearchSession> sessions;
    int current_index = -1;

    // 新しい探索セッションを追加
    void push_session(const SearchSession& session) {
        // 過去の履歴に戻っている状態で新規 Search された場合、それ以降の未来履歴をカット
        if (current_index >= 0 && current_index + 1 < static_cast<int>(sessions.size())) {
            sessions.erase(sessions.begin() + current_index + 1, sessions.end());
        }
        sessions.push_back(session);
        current_index = static_cast<int>(sessions.size()) - 1;
    }

    bool can_undo() const { return current_index > 0; }
    bool can_redo() const { return current_index >= 0 && current_index + 1 < static_cast<int>(sessions.size()); }

    void undo() { if (can_undo()) current_index--; }
    void redo() { if (can_redo()) current_index++; }

    const SearchSession* current_session() const {
        if (current_index >= 0 && current_index < static_cast<int>(sessions.size())) {
            return &sessions[current_index];
        }
        return nullptr;
    }
};

#endif // QYKIS2_UTILS_HISTORY_MANAGER_HPP