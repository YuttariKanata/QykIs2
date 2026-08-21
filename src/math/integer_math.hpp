#ifndef QYKIS2_MATH_INTEGER_MATH_HPP
#define QYKIS2_MATH_INTEGER_MATH_HPP

#include <gmp.h>
#include <gmpxx.h>
#include <cmath>
#include <cstdint>
#include <optional>
#include <atcoder/modint>
#include <vector>

#if defined(__SIZEOF_INT128__)
    using int128_t = __int128_t;
    using uint128_t = __uint128_t;
#else
    #error "128-bit integer support (__int128_t) is required for QykIs2 Engine."
#endif

// __int128_t / int128_t 用の非負剰余関数
inline constexpr int mod_p(int128_t val, int p) noexcept {
    int128_t m = val % p;
    if (m < 0) m += p;
    return static_cast<int>(m);
}

// d の相異なる素因数リストを取得するヘルパー
static std::vector<int64_t> get_prime_factors(int64_t n) {
    std::vector<int64_t> factors;
    int64_t temp = n;
    for (int64_t p = 2; p * p <= temp; ++p) {
        if (temp % p == 0) {
            factors.push_back(p);
            while (temp % p == 0) temp /= p;
        }
    }
    if (temp > 1) factors.push_back(temp);
    return factors;
}

// --------------------------------------------------
// 超高速 128bit bit / isqrt モジュール
// --------------------------------------------------

static inline int clz128(uint128_t x) {
    uint64_t high = static_cast<uint64_t>(x >> 64);
    uint64_t low = static_cast<uint64_t>(x);
    if (high == 0) {
        return 64 + __builtin_clzll(low);
    }
    return __builtin_clzll(high);
}

inline uint64_t isqrt64(uint64_t n) {
    if (n <= 1) return n;
    uint64_t x = static_cast<uint64_t>(std::sqrt(static_cast<double>(n)));
    while (x < UINT32_MAX && (x + 1) * (x + 1) <= n) x++;
    while (x > 0 && (x > UINT32_MAX || x * x > n)) x--;
    return x;
}

inline uint128_t isqrt128(uint128_t n) {
    if (n <= 1) return n;

    int lz = clz128(n);
    if (lz >= 64) return isqrt64(static_cast<uint64_t>(n));

    int a = (lz == 2) ? 1 : (lz - 1) / 2;

    uint128_t scaled_n = n << (2 * a);
    uint64_t u = static_cast<uint64_t>(scaled_n >> 64);
    uint64_t isqu = isqrt64(u);

    uint128_t x = ((static_cast<uint128_t>(isqu) << 32) + ((((u - isqu * isqu) << 31) | (static_cast<uint64_t>(scaled_n) >> 33)) / isqu)) >> a;

    while (x < UINT64_MAX && (x + 1) * (x + 1) <= n) x++;
    while (x > 0 && (x > UINT64_MAX || x * x > n)) x--;
    return x;
}

// --------------------------------------------------
// Mod 256 プリフィルター & 完全平方数判定
// --------------------------------------------------

alignas(64) static const uint8_t SQ_MOD256_LOOKUP[256] = {
    1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
};

inline std::optional<int128_t> check_perfect_square(int128_t n) {
    if (n < 0) return std::nullopt;
    if (n == 0) return 0;

    const uint8_t mod256 = static_cast<uint8_t>(static_cast<uint64_t>(n) & 0xFF);
    if (!SQ_MOD256_LOOKUP[mod256]) {
        return std::nullopt;
    }

    const uint128_t un = static_cast<uint128_t>(n);
    const uint128_t r = isqrt128(un);

    if (r * r == un) {
        return static_cast<int128_t>(r);
    }

    return std::nullopt;
}

// --------------------------------------------------
// GMP mpz_t/mpz_class <---> int128_t 安全変換
// --------------------------------------------------
inline bool mpz_class_to_int128(int128_t& dst, const mpz_class& src) {
    if (mpz_sizeinbase(src.get_mpz_t(), 2) > 127) {
        return false;
    }

    size_t count = 0;
    uint64_t limbs[2] = {0, 0};
    mpz_export(limbs, &count, -1, sizeof(uint64_t), 0, 0, src.get_mpz_t());

    int128_t val = 0;
    if (count > 0) val |= static_cast<int128_t>(limbs[0]);
    if (count > 1) val |= (static_cast<int128_t>(limbs[1]) << 64);

    if (src < 0) {
        val = -val;
    }

    dst = val;
    return true;
}

// int128_t を mpz_class へ安全にセットするヘルパー
static inline void int128_to_mpz_class(mpz_class& dst, int128_t op) {
    if (op == 0) {
        dst = 0;
        return;
    }

    bool is_negative = (op < 0);
    uint128_t abs_op = is_negative ? static_cast<uint128_t>(-op) : static_cast<uint128_t>(op);

    uint64_t limbs[2];
    limbs[0] = static_cast<uint64_t>(abs_op);
    limbs[1] = static_cast<uint64_t>(abs_op >> 64);

    size_t count = (limbs[1] != 0) ? 2 : 1;
    
    // mpz_class の内部 mpz_t に直接 import
    mpz_import(dst.get_mpz_t(), count, -1, sizeof(uint64_t), 0, 0, limbs);

    if (is_negative) {
        dst = -dst;
    }
}

#endif // QYKIS2_MATH_INTEGER_MATH_HPP