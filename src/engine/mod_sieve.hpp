#ifndef QYKIS2_ENGINE_MOD_SIEVE_HPP
#define QYKIS2_ENGINE_MOD_SIEVE_HPP

#include <vector>
#include <cstdint>
#include <atcoder/modint>

// エラトステネスの篩による素数生成
inline std::vector<int> generate_primes(int limit) {
    if (limit < 2) return {};
    std::vector<bool> is_prime(limit + 1, true);
    is_prime[0] = is_prime[1] = false;
    for (int p = 2; p * p <= limit; ++p) {
        if (is_prime[p]) {
            for (int i = p * p; i <= limit; i += p) {
                is_prime[i] = false;
            }
        }
    }
    std::vector<int> primes;
    for (int p = 2; p <= limit; ++p) {
        if (is_prime[p]) primes.push_back(p);
    }
    return primes;
}

class ModSieve {
public:
    explicit ModSieve(int prime_limit = 50) {
        primes_ = generate_primes(prime_limit);
        
        for (int p : primes_) {
            using mint = atcoder::modint;
            mint::set_mod(p);

            std::vector<bool> table(p, false);
            for (int y = 0; y < p; ++y) {
                mint my = y;
                table[(my * my).val()] = true;
            }
            is_sq_tables_.push_back(std::move(table));
        }
    }

    // 3次曲線: Y^2 = X^3 + a*d^4*X + b*d^6 (mod p)
    inline bool passes_deg3(int64_t a, int64_t b, int64_t X, int64_t d) const {
        using mint = atcoder::modint;

        for (size_t i = 0; i < primes_.size(); ++i) {
            const int p = primes_[i];
            mint::set_mod(p);

            // ACL の modint コンストラクタは負の数も正しく mod p に正規化してくれてうれしい
            const mint mx = X;
            const mint md = d;
            const mint ma = a;
            const mint mb = b;

            const mint md2 = md * md;
            const mint md4 = md2 * md2;
            const mint md6 = md4 * md2;

            const mint rhs = mx * mx * mx + ma * md4 * mx + mb * md6;

            // 平方非剰余おとし
            if (!is_sq_tables_[i][rhs.val()]) {
                return false;
            }
        }
        return true;
    }

    // 4次曲線: Y^2 = X^4 + a*X^2*d^2 + b*X*d^3 + c*d^4 (mod p)
    inline bool passes_deg4(int64_t a, int64_t b, int64_t c, int64_t X, int64_t d) const {
        using mint = atcoder::modint;

        for (size_t i = 0; i < primes_.size(); ++i) {
            const int p = primes_[i];
            mint::set_mod(p);

            const mint mx = X;
            const mint md = d;
            const mint ma = a;
            const mint mb = b;
            const mint mc = c;

            const mint mx2 = mx * mx;
            const mint mx4 = mx2 * mx2;

            const mint md2 = md * md;
            const mint md3 = md2 * md;
            const mint md4 = md2 * md2;

            const mint rhs = mx4 + ma * mx2 * md2 + mb * mx * md3 + mc * md4;

            // 平方非剰余おとし
            if (!is_sq_tables_[i][rhs.val()]) {
                return false;
            }
        }
        return true;
    }

private:
    std::vector<int> primes_;
    std::vector<std::vector<bool>> is_sq_tables_;
};

#endif // QYKIS2_ENGINE_MOD_SIEVE_HPP