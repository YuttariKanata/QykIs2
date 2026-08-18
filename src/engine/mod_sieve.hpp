#ifndef QYKIS2_ENGINE_MOD_SIEVE_HPP
#define QYKIS2_ENGINE_MOD_SIEVE_HPP

#include <vector>
#include <cstdint>
#include <atcoder/modint>

// エラトステネスの篩
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
private:
    std::vector<int> primes_;
    std::vector<std::vector<bool>> is_sq_tables_;

public:

    const std::vector<int>& primes() const { return primes_; }

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

            const mint rhs = mx*mx*mx + (ma*mx + mb*md2) * md4;

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

            const mint rhs = ((mc*md + mb*mx) * md + ma*mx2) * md2 + mx4;

            // 平方非剰余おとし
            if (!is_sq_tables_[i][rhs.val()]) {
                return false;
            }
        }
        return true;
    }

    // 5次曲線: Y^2 = X^5 + aX^3*de^4 + bX^2*de^6 + cX*de^8 + d*de^10 (mod p)
    // Y^2 = X^5 + de^4*(aX^3 + de^2*(bX^2 + de^2*(cX + d*de^2))) (mod p)
    inline bool passes_deg5(int64_t a, int64_t b, int64_t c, int64_t d, int64_t X, int64_t de) const {
        using mint = atcoder::modint;

        for (size_t i = 0; i < primes_.size(); ++i) {
            const int p = primes_[i];
            mint::set_mod(p);

            const mint mx = X;
            const mint mde = de;
            const mint ma = a;
            const mint mb = b;
            const mint mc = c;
            const mint md = d;

            const mint mx2 = mx*mx;
            const mint mx3 = mx2*mx;
            const mint mx5 = mx3*mx2;

            const mint mde2 = mde*mde;
            const mint mde4 = mde2*mde2;

            const mint rhs = (((md*mde2 + mc*mx) * mde2 + mb*mx2) * mde2 + ma*mx3) * mde4 + mx5;

            // 平方非剰余おとし
            if (!is_sq_tables_[i][rhs.val()]) {
                return false;
            }
        }
        return true;
    }
};

#endif // QYKIS2_ENGINE_MOD_SIEVE_HPP