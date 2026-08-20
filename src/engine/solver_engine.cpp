#include "engine/solver_engine.hpp"
#include <utility>

void SolverEngine::start_search(
    const StandardCurveConfig& config,
    const CurveTransformInfo& transform,
    int64_t max_d,
    int64_t max_X
) {
    // 既に探索が動いている場合は一度確実に停止・ジョインする
    stop_search();

    // フラグおよび進捗の初期化
    m_stop_requested.store(false);
    m_is_searching.store(true);
    m_progress.store(0.0);

    // ワーカースレッドの起動 (パラメータはスレッドローカルへコピー渡し)
    m_worker = std::thread(
        &SolverEngine::worker_thread,
        this,
        config,
        transform,
        max_d,
        max_X
    );
}

void SolverEngine::stop_search() {
    // 停止フラグを立ててワーカースレッドのループ脱出を促す
    m_stop_requested.store(true);

    if (m_worker.joinable()) {
        m_worker.join();
    }

    m_is_searching.store(false);
}

// --------------------------------------------------
// 以下、worker_thread の仮置き（次のステップで完全実装）
// --------------------------------------------------
void SolverEngine::worker_thread(
    StandardCurveConfig config,
    CurveTransformInfo transform,
    int64_t max_d,
    int64_t max_X
) {
    // TODO: ModSieve の初期化および d, X の探索ループ
    // 解検出時の処理イメージ:
    // mpz_class ox, oy;
    // if (map_point_to_original(u, v, transform, ox, oy)) {
    //     PointCallback cb;
    //     {
    //         std::lock_guard<std::mutex> lock(m_cb_mutex);
    //         cb = m_on_point_found;
    //     }
    //     if (cb) cb({ox, oy, u, v});
    // }

    m_is_searching.store(false);
}