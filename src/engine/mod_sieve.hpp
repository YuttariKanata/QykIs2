#ifndef QYKIS2_ENGINE_MOD_SIEVE_HPP
#define QYKIS2_ENGINE_MOD_SIEVE_HPP

#include "math/curve_transform.hpp"
#include "math/number_theory.hpp"
#include <atcoder/modint>
#include <cstdint>
#include <vector>

class ModSieve {
private:
    std::vector<int> primes_;
    
    // valid_x_tables_[i][X % p] : 
    // 現在の D において X ≡ X_mod_p (mod p) のとき Y^2 ≡ T(X, D) (mod p) に解が存在すれば true
    std::vector<std::vector<bool>> valid_x_tables_;

    static inline int mod_int128(int128_t val, int p) {
        int128_t m = val % p;
        if (m < 0) m += p;
        return static_cast<int>(m);
    }

public:
    ModSieve() = default;

    const std::vector<int>& primes() const { return primes_; }

    /**
     * @brief D (または de) を固定した段階で呼び出し、その D に対する X (mod p) テーブルを構築する
     */
    void init_for_D(const StandardCurveConfig& config, int64_t D, int prime_limit = 50) {
        if (primes_.empty()) {
            primes_ = generate_primes(prime_limit);
        }
        valid_x_tables_.resize(primes_.size());

        for (size_t i = 0; i < primes_.size(); ++i) {
            const int p = primes_[i];
            using mint = atcoder::modint;
            mint::set_mod(p);

            // 1. mod p での平方余剰フラグテーブル作成
            std::vector<bool> is_sq(p, false);
            for (int y = 0; y < p; ++y) {
                mint my = y;
                is_sq[(my * my).val()] = true;
            }

            std::vector<bool> table(p, false);

            if (config.degree == 3) {
                // 3次: Y^2 = X^3 + (a*X + b*d^2) * d^4
                const mint ma = mod_int128(config.coeff_c3, p);
                const mint mb = mod_int128(config.coeff_c4, p);
                const mint md = D;

                const mint md2 = md * md;
                const mint md4 = md2 * md2;
                const mint mb_md2 = mb * md2; // D依存定数を外で事前計算

                for (int rx = 0; rx < p; ++rx) {
                    const mint mx = rx;
                    const mint rhs = mx * mx * mx + (ma * mx + mb_md2) * md4;
                    table[rx] = is_sq[rhs.val()];
                }
            } else if (config.degree == 4) {
                // 4次: Y^2 = (((c*d + b*X)*d + a*X^2)*d^2 + X^4)
                const mint ma = mod_int128(config.coeff_c3, p);
                const mint mb = mod_int128(config.coeff_c4, p);
                const mint mc = mod_int128(config.coeff_c5, p);
                const mint md = D;

                const mint md2 = md * md;
                const mint mc_md = mc * md; // D依存定数を外で事前計算

                for (int rx = 0; rx < p; ++rx) {
                    const mint mx = rx;
                    const mint mx2 = mx * mx;
                    const mint mx4 = mx2 * mx2;

                    const mint rhs = ((mc_md + mb * mx) * md + ma * mx2) * md2 + mx4;
                    table[rx] = is_sq[rhs.val()];
                }
            } else if (config.degree == 5) {
                // 5次: Y^2 = (((d*de^2 + c*X)*de^2 + b*X^2)*de^2 + a*X^3)*de^4 + X^5
                const mint ma = mod_int128(config.coeff_c3, p);
                const mint mb = mod_int128(config.coeff_c4, p);
                const mint mc = mod_int128(config.coeff_c5, p);
                const mint md_coeff = mod_int128(config.coeff_c6, p);
                const mint mde = D;

                const mint mde2 = mde * mde;
                const mint mde4 = mde2 * mde2;
                const mint md_mde2 = md_coeff * mde2; // D依存定数を外で事前計算

                for (int rx = 0; rx < p; ++rx) {
                    const mint mx = rx;
                    const mint mx2 = mx * mx;
                    const mint mx3 = mx2 * mx;
                    const mint mx5 = mx3 * mx2;

                    const mint rhs = (((md_mde2 + mc * mx) * mde2 + mb * mx2) * mde2 + ma * mx3) * mde4 + mx5;
                    table[rx] = is_sq[rhs.val()];
                }
            }

            valid_x_tables_[i] = std::move(table);
        }
    }

    /**
     * @brief X が現在の D において候補になり得るか O(1) 判定
     */
    inline bool is_candidate(int64_t X) const {
        for (size_t i = 0; i < primes_.size(); ++i) {
            const int p = primes_[i];
            int mod_x = static_cast<int>(X % p);
            if (mod_x < 0) mod_x += p;

            if (!valid_x_tables_[i][mod_x]) {
                return false;
            }
        }
        return true;
    }
};

#endif // QYKIS2_ENGINE_MOD_SIEVE_HPP