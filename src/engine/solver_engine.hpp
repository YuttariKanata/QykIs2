#ifndef ENGINE_SOLVER_ENGINE_HPP
#define ENGINE_SOLVER_ENGINE_HPP

#include "math/curve_transform.hpp"
#include <gmpxx.h>
#include <atomic>
#include <thread>
#include <vector>
#include <functional>
#include <mutex>
#include <cstdint>

// 検出された解の情報を保持する構造体
struct FoundPoint {
    mpz_class x;
    mpz_class y;
    int128_t u; // 標準形上の X (デバッグ・ログ用)
    int128_t v; // 標準形上の Y (デバッグ・ログ用)
};

class SolverEngine {
public:
    using PointCallback = std::function<void(const FoundPoint&)>;

    SolverEngine() = default;
    ~SolverEngine() { stop_search(); }

    // コピー不可
    SolverEngine(const SolverEngine&) = delete;
    SolverEngine& operator=(const SolverEngine&) = delete;

    /**
     * @brief 探索を開始する
     * 
     * @param config 正規化された整係数曲線の設定 (Y^2 = A * f(x))
     * @param transform 逆変換用情報 (y = Y / scale_A)
     * @param max_d 探索境界 (分母・パラメータ等)
     * @param max_X 探索 X 範囲 [-max_X, max_X]
     */
    void start_search(
        const StandardCurveConfig& config,
        const CurveTransformInfo& transform,
        int64_t max_d,
        int64_t max_X
    );

    /**
     * @brief 探索を強制停止する
     */
    void stop_search();

    /**
     * @brief 現在探索中かどうか
     */
    bool is_searching() const { return m_is_searching.load(); }

    /**
     * @brief 進行状況 (0.0 ～ 1.0) の取得
     */
    double get_progress() const { return m_progress.load(); }

    /**
     * @brief 解が見つかった際につぶやくコールバックの登録
     */
    void set_on_point_found(PointCallback cb) {
        std::lock_guard<std::mutex> lock(m_cb_mutex);
        m_on_point_found = std::move(cb);
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

    std::mutex m_cb_mutex;
    PointCallback m_on_point_found;
};

#endif // ENGINE_SOLVER_ENGINE_HPP