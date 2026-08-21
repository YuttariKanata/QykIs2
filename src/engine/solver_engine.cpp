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

#include <numeric>
#include <vector>
#include <chrono>
#include <iostream>
#include <iomanip>


void SolverEngine::worker_thread(
    StandardCurveConfig config,
    CurveTransformInfo transform,
    int64_t max_d,
    int64_t max_X
) {
    m_is_searching.store(true); // こんに

    ModSieve sieve;

    auto push_point = [this](int128_t u, int128_t v, int64_t d) {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        m_found_queue.push_back({u, v, d});
    };

    const auto start_time = std::chrono::high_resolution_clock::now();
    uint64_t total_evaluations = 0;

    // =========================================================================
    // 【分岐 1】 4次曲線専用ループ (x = X/d, gcd(X, d) = 1)
    // =========================================================================
    if (config.degree == 4) {
        std::vector<uint8_t> is_coprime; // d 周期の gcd テーブル

        for (int64_t d = 1; d <= max_d; ++d) {
            if (m_stop_requested.load(std::memory_order_relaxed)) break;
            m_progress.store(static_cast<double>(d) / static_cast<double>(max_d));
            sieve.init_for_D(config, d);

            // d 周期の互いに素テーブルを事前構築 (gcd 計算は d 回のみ)
            is_coprime.assign(d, 0);
            for (int64_t r = 0; r < d; ++r) {
                if (std::gcd(r, d) == 1) {
                    is_coprime[r] = 1;
                }
            }

            for (int64_t X = -max_X; X <= max_X; ++X) {
                // X % d を正の剰余に正規化して高速テーブル参照
                int64_t r = X % d;
                if (r < 0) r += d;
                
                if (!is_coprime[r]) continue; // gcd(X, d) != 1 を超高速スキップ
                total_evaluations++;

                if (!sieve.is_candidate(X)) continue;

                auto opt_Y = eval_exact_deg4(config, X, d);
                if (opt_Y.has_value()) {
                    const int128_t Y = opt_Y.value();
                    push_point(X, Y, d);
                    if (Y != 0) push_point(X, -Y, d);
                }
            }
        }
    } else {
        // =========================================================================
        // 【分岐 2】 3次・5次曲線専用ループ (x = X/d^2, p^2 | X をスキップ)
        // =========================================================================
        for (int64_t d = 1; d <= max_d; ++d) {
            if (m_stop_requested.load(std::memory_order_relaxed)) break;
            m_progress.store(static_cast<double>(d) / static_cast<double>(max_d));
            sieve.init_for_D(config, d);

            // p^2 の倍数リストを作成
            const auto primes = get_prime_factors(d);
            std::vector<int64_t> bad_p_sq;
            bad_p_sq.reserve(primes.size());
            for (int64_t p : primes) {
                bad_p_sq.push_back(p * p);
            }

            for (int64_t X = -max_X; X <= max_X; ++X) {
                // p^2 | X チェック
                bool is_redundant = false;
                for (int64_t p_sq : bad_p_sq) {
                    if (X % p_sq == 0) {
                        is_redundant = true;
                        break;
                    }
                }
                if (is_redundant) continue;
                total_evaluations++;

                if (!sieve.is_candidate(X)) continue;

                std::optional<int128_t> opt_Y;
                if (config.degree == 3) {
                    opt_Y = eval_exact_deg3(config, X, d);
                } else {
                    opt_Y = eval_exact_deg5(config, X, d);
                }

                if (opt_Y.has_value()) {
                    const int128_t Y = opt_Y.value();
                    push_point(X, Y, d);
                    if (Y != 0) push_point(X, -Y, d);
                }
            }
        }
    }

    const auto end_time = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> elapsed = end_time - start_time;
    const double seconds = elapsed.count();
    const double m_iters = (seconds > 0.0) ? (static_cast<double>(total_evaluations) / seconds / 1e6) : 0.0;

    std::cerr << "\n================ [ Search Benchmark (Separated Order) ] ================\n"
              << " Degree       : " << config.degree << "\n"
              << " Range        : d <= " << max_d << ", |X| <= " << max_X << "\n"
              << " Evaluated X  : " << total_evaluations << " points\n"
              << " Elapsed Time : " << std::fixed << std::setprecision(4) << seconds << " sec\n"
              << " Throughput   : " << std::fixed << std::setprecision(2) << m_iters << " M_evals/sec\n"
              << "========================================================================\n" << std::endl;

    m_progress.store(1.0);
    m_is_searching.store(false);
}