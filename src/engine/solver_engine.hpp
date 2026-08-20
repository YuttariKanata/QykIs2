#ifndef QYKIS2_ENGINE_SOLVER_ENGINE_HPP
#define QYKIS2_ENGINE_SOLVER_ENGINE_HPP

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstdint>
#include "math/curve_eval.hpp"

enum class CurveDegree {
    Degree3,
    Degree4,
    Degree5
};

struct CurveConfig {
    CurveDegree degree = CurveDegree::Degree3;
    int64_t a = 0;
    int64_t b = 0;
    int64_t c = 0;
    int64_t d_coeff = 0;
};

struct RationalPoint {
    int128_t num_x, den_x;
    int128_t num_y, den_y;
};

class SolverEngine {
public:
    SolverEngine();
    ~SolverEngine();

    void start_search(const CurveConfig& config, int64_t max_d, int64_t max_X);
    void stop_search();

    std::vector<RationalPoint> pop_new_points();

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