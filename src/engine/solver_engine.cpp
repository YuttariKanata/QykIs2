#include "engine/solver_engine.hpp"
#include "engine/mod_sieve.hpp"
#include <numeric>

SolverEngine::SolverEngine() = default;

SolverEngine::~SolverEngine() {
    stop_search();
}

void SolverEngine::start_search(const CurveConfig& config, int64_t max_d, int64_t max_X) {
    stop_search(); // すでに走っていれば安全に停止

    is_running_ = true;
    stop_requested_ = false;
    current_d_ = 0;
    total_checked_ = 0;

    {
        std::lock_guard<std::mutex> lock(points_mutex_);
        found_points_.clear();
    }

    worker_thread_ = std::thread(&SolverEngine::worker_loop, this, config, max_d, max_X);
}

void SolverEngine::stop_search() {
    if (is_running_) {
        stop_requested_ = true;
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
        is_running_ = false;
    }
}

std::vector<RationalPoint> SolverEngine::pop_new_points() {
    std::lock_guard<std::mutex> lock(points_mutex_);
    std::vector<RationalPoint> result;
    result.swap(found_points_); // Lock時間を最小限にするため swap
    return result;
}

void SolverEngine::worker_loop(CurveConfig config, int64_t max_d, int64_t max_X) {
    ModSieve sieve(50); // 小素数 p <= 50 の余りテーブルを自動準備
    uint64_t checked_count = 0;

    for (int64_t d = 1; d <= max_d && !stop_requested_; ++d) {
        current_d_ = d;

        const int64_t d2 = d * d;
        const int64_t d3 = d2 * d;
        const int64_t d4 = d2 * d2;
        const int64_t d5 = d3 * d2;

        for (int64_t X = -max_X; X <= max_X && !stop_requested_; ++X) {
            checked_count++;

            // [チェックA] 既約分数カット (gcd(X, d) == 1)
            if (std::gcd(X, d) != 1) continue;

            // [チェックB] Mod Sieve フィルター (余り計算のみで98%超撃墜)
            bool pass_sieve = false;
            switch (config.degree) {
                case CurveDegree::Degree3:
                    pass_sieve = sieve.passes_deg3(config.a, config.b, X, d);
                    break;
                case CurveDegree::Degree4:
                    pass_sieve = sieve.passes_deg4(config.a, config.b, config.c, X, d);
                    break;
                case CurveDegree::Degree5:
                    pass_sieve = sieve.passes_deg5(config.a, config.b, config.c, config.d_coeff, X, d);
                    break;
            }
            if (!pass_sieve) continue;

            // [チェックC] T(X, d) の 128bit 厳密評価 & isqrt128 完全平方数判定
            std::optional<int128_t> opt_Y;
            switch (config.degree) {
                case CurveDegree::Degree3:
                    opt_Y = eval_exact_deg3(config.a, config.b, X, d);
                    break;
                case CurveDegree::Degree4:
                    opt_Y = eval_exact_deg4(config.a, config.b, config.c, X, d);
                    break;
                case CurveDegree::Degree5:
                    opt_Y = eval_exact_deg5(config.a, config.b, config.c, config.d_coeff, X, d);
                    break;
            }

            // 有理点発見！
            if (opt_Y.has_value()) {
                const int128_t Y = opt_Y.value();

                // 同次座標の分母スケールを復元
                // 3次: x = X/d^2, y = Y/d^3
                // 4次: x = X/d,   y = Y/d^2
                // 5次: x = X/d^2, y = Y/d^5 (※5次の標準同次形に対応)
                int128_t den_x = 1, den_y = 1;
                if (config.degree == CurveDegree::Degree3) {
                    den_x = d2; den_y = d3;
                } else if (config.degree == CurveDegree::Degree4) {
                    den_x = d;  den_y = d2;
                } else if (config.degree == CurveDegree::Degree5) {
                    den_x = d2; den_y = d5;
                }

                std::lock_guard<std::mutex> lock(points_mutex_);
                found_points_.push_back({X, den_x, Y, den_y});
                if (Y != 0) {
                    // y 軸対称点 (x, -y) も同時に記録
                    found_points_.push_back({X, den_x, -Y, den_y});
                }
            }
        }

        total_checked_ += checked_count;
        checked_count = 0;
    }

    is_running_ = false;
}