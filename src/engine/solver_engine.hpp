#ifndef QYKIS2_ENGINE_SOLVER_ENGINE_HPP
#define QYKIS2_ENGINE_SOLVER_ENGINE_HPP

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstdint>
#include "engine/exact_eval.hpp"

// 曲線の次数・種類
enum class CurveDegree {
    Degree3, // Y^2 = X^3 + a*d^4*X + b*d^6
    Degree4, // Y^2 = X^4 + a*X^2*d^2 + b*X*d^3 + c*d^4
    Degree5  // Y^2 = X^5 + a*X^3*d^4 + b*X^2*d^6 + c*X*d^8 + d_coeff*d^10
};

// 曲線パラメータ
struct CurveConfig {
    CurveDegree degree = CurveDegree::Degree3;
    int64_t a = 0;
    int64_t b = 0;
    int64_t c = 0;
    int64_t d_coeff = 0;
};

// 発見した有理点 (x = num_x / den_x, y = num_y / den_y)
struct RationalPoint {
    int128_t num_x, den_x;
    int128_t num_y, den_y;
};

class SolverEngine {
public:
    SolverEngine();
    ~SolverEngine();

    // 探索の開始・停止
    void start_search(const CurveConfig& config, int64_t max_d, int64_t max_X);
    void stop_search();

    // 新たに発見された有理点をメインスレッド（GUI）へ取得＆クリア
    std::vector<RationalPoint> pop_new_points();

    // ステータス取得
    bool is_running() const { return is_running_.load(); }
    int64_t current_d() const { return current_d_.load(); }
    uint64_t total_checked() const { return total_checked_.load(); }

private:
    void worker_loop(CurveConfig config, int64_t max_d, int64_t max_X);

    std::atomic<bool> is_running_{false};
    std::atomic<bool> stop_requested_{false};
    std::atomic<int64_t> current_d_{0};
    std::atomic<uint64_t> total_checked_{0};

    std::mutex points_mutex_;
    std::vector<RationalPoint> found_points_;
    std::thread worker_thread_;
};

#endif // QYKIS2_ENGINE_SOLVER_ENGINE_HPP