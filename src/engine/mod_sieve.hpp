#ifndef QYKIS2_ENGINE_MOD_SIEVE_HPP
#define QYKIS2_ENGINE_MOD_SIEVE_HPP

#include "math/curve_transform.hpp"
#include "math/number_theory.hpp"
#include <atcoder/modint>
#include <cstdint>
#include <vector>

class ModSieve {
private:
    using mint = atcoder::modint; // クラス内で使用する型エイリアス

    std::vector<int> primes_;
    
    // valid_x_tables_[i][X % p] : 
    // 現在の D において X ≡ X_mod_p (mod p) のとき Y^2 ≡ T(X, D) (mod p) に解が存在すれば true
    std::vector<std::vector<bool>> valid_x_tables_;

public:
    ModSieve() = default;

    const std::vector<int>& primes() const { return primes_; }

    /**
     * @brief D を固定した段階で呼び出し、その D に対する X (mod p) テーブルを構築する
     */
    void init_for_D(const StandardCurveConfig& config, int64_t D, int prime_limit = 50) {
        if (primes_.empty()) {
            primes_ = generate_primes(prime_limit);
        }
        valid_x_tables_.resize(primes_.size());

        for (size_t i = 0; i < primes_.size(); ++i) {
            const int p = primes_[i];
            mint::set_mod(p);

            // 1. mod p での平方余剰フラグ
            std::vector<bool> is_sq(p, false);
            for (int y = 0; y < p; ++y) {
                mint my = mint::raw(y);
                is_sq[(my * my).val()] = true;
            }

            // 2. D の冪乗
            const mint mD = D;
            const mint mD2 = mD * mD;
            std::vector<bool> table(p, false);

            if (config.degree == 3) {
                const mint mD4 = mD2 * mD2;
                const mint mD6 = mD4 * mD2;

                // 追記した 128-bit コンストラクタにより直入れが可能
                const mint a = config.coeff_x_max;
                const mint b = config.coeff_c2 * mD2;
                const mint c = config.coeff_c3 * mD4;
                const mint d = config.coeff_c4 * mD6;

                for (int rx = 0; rx < p; ++rx) {
                    const mint mX = mint::raw(rx);
                    const mint rhs = ((a * mX + b) * mX + c) * mX + d;
                    table[rx] = is_sq[rhs.val()];
                }
            } else if (config.degree == 4) {
                const mint mD3 = mD2 * mD;
                const mint mD4 = mD2 * mD2;

                const mint a = config.coeff_x_max;
                const mint b = config.coeff_c2 * mD;
                const mint c = config.coeff_c3 * mD2;
                const mint d = config.coeff_c4 * mD3;
                const mint e = config.coeff_c5 * mD4;

                for (int rx = 0; rx < p; ++rx) {
                    const mint mX = mint::raw(rx);
                    const mint rhs = (((a * mX + b) * mX + c) * mX + d) * mX + e;
                    table[rx] = is_sq[rhs.val()];
                }
            } else if (config.degree == 5) {
                const mint mD4 = mD2 * mD2;
                const mint mD6 = mD4 * mD2;
                const mint mD8 = mD4 * mD4;
                const mint mD10 = mD8 * mD2;

                const mint a = config.coeff_x_max;
                const mint b = config.coeff_c2 * mD2;
                const mint c = config.coeff_c3 * mD4;
                const mint d = config.coeff_c4 * mD6;
                const mint e = config.coeff_c5 * mD8;
                const mint f = config.coeff_c6 * mD10;

                for (int rx = 0; rx < p; ++rx) {
                    const mint mX = mint::raw(rx);
                    const mint rhs = ((((a * mX + b) * mX + c) * mX + d) * mX + e) * mX + f;
                    table[rx] = is_sq[rhs.val()];
                }
            }

            valid_x_tables_[i] = std::move(table);
        }
    }

    /**
     * @brief X が現在の D において候補になり得るか判定する (熱い内側ループ用)
     */
    inline bool is_candidate(int64_t X) const {
        const size_t num_primes = primes_.size();
        for (size_t i = 0; i < num_primes; ++i) {
            const int p = primes_[i];

            // 負数 X に対する modulo p 計算の最適化
            // (多くのコンパイラで % 命令は重いため、符号制御を最小化)
            int mod_x = static_cast<int>(X % p);
            if (mod_x < 0) mod_x += p;

            // vector<bool> へのアクセス
            if (!valid_x_tables_[i][mod_x]) {
                return false;
            }
        }
        return true;
    }
};

#endif // QYKIS2_ENGINE_MOD_SIEVE_HPP