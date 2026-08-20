#ifndef ENGINE_SOLVER_ENGINE_HPP
#define ENGINE_SOLVER_ENGINE_HPP

#include "math/curve_transform.hpp"
#include <atomic>
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>

// 探索スレッドが見つけた標準形上の生の座標 (u, v)
struct FoundPoint {
    int128_t u; // X
    int128_t v; // Y
    int64_t  d; // 分母パラメータ d
};

class SolverEngine {
public:
    SolverEngine() = default;
    ~SolverEngine() { stop_search(); }

    SolverEngine(const SolverEngine&) = delete;
    SolverEngine& operator=(const SolverEngine&) = delete;

    void start_search(
        const StandardCurveConfig& config,
        const CurveTransformInfo& transform,
        int64_t max_d,
        int64_t max_X
    );

    void stop_search();

    bool is_searching() const { return m_is_searching.load(); }
    double get_progress() const { return m_progress.load(); }

    // ワーカースレッドが溜めた (u, v) を UI スレッド側から一括回収する
    std::vector<FoundPoint> pop_found_points() {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        std::vector<FoundPoint> result;
        result.swap(m_found_queue);
        return result;
    }

private:
    void worker_thread(
        StandardCurveConfig config,
        CurveTransformInfo transform,
        int64_t max_d,
        int64_t max_X
    );

    std::thread m_worker;
    std::atomic<bool> m_is_searching{false};
    std::atomic<bool> m_stop_requested{false};
    std::atomic<double> m_progress{0.0};

    std::mutex m_queue_mutex;
    std::vector<FoundPoint> m_found_queue;
};

#endif // ENGINE_SOLVER_ENGINE_HPP