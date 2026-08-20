#include "engine/solver_engine.hpp"
#include "math/curve_eval.hpp"
#include "math/integer_math.hpp"
#include "engine/mod_sieve.hpp"

void SolverEngine::start_search(
    const StandardCurveConfig& config,
    const CurveTransformInfo& transform,
    int64_t max_d,
    int64_t max_X
) {
    stop_search();

    m_stop_requested.store(false);
    m_progress.store(0.0);

    {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        m_found_queue.clear();
    }

    m_worker = std::thread(&SolverEngine::worker_thread, this, config, transform, max_d, max_X);
}

void SolverEngine::stop_search() {
    m_stop_requested.store(true);
    if (m_worker.joinable()) {
        m_worker.join();
    }
    m_is_searching.store(false);
}

void SolverEngine::worker_thread(
    StandardCurveConfig config,
    CurveTransformInfo transform,
    int64_t max_d,
    int64_t max_X
) {
    m_is_searching.store(true);

    ModSieve sieve;

    for (int64_t d = 1; d <= max_d; ++d) {
        if (m_stop_requested.load(std::memory_order_relaxed)) {
            break;
        }

        // 発見した (u, v) を安全にキューへ追加するヘルパー
        auto push_point = [this, d](int128_t u, int128_t v) {
            std::lock_guard<std::mutex> lock(m_queue_mutex);
            m_found_queue.push_back({u, v, d});
        };

        m_progress.store(static_cast<double>(d) / static_cast<double>(max_d));

        // モジュラふるいの初期化
        sieve.init_for_D(config, d);

        for (int64_t X = -max_X; X <= max_X; ++X) {
            if (!sieve.is_candidate(X)) {
                continue;
            }

            std::optional<int128_t> opt_Y;
            if (config.degree == 3) {
                opt_Y = eval_exact_deg3(config, X, d);
            } else if (config.degree == 4) {
                opt_Y = eval_exact_deg4(config, X, d);
            } else if (config.degree == 5) {
                opt_Y = eval_exact_deg5(config, X, d);
            }

            if (opt_Y.has_value()) {
                const int128_t Y = opt_Y.value();
                push_point(X, Y);
                if (Y != 0) {
                    push_point(X, -Y);
                }
            }
        }
    }

    m_progress.store(1.0);
    m_is_searching.store(false);
}