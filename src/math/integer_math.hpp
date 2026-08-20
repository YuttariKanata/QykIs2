#ifndef QYKIS2_MATH_INTEGER_MATH_HPP
#define QYKIS2_MATH_INTEGER_MATH_HPP

#include <cmath>
#include <cstdint>
#include <optional>

#if defined(__SIZEOF_INT128__)
    using int128_t = __int128_t;
    using uint128_t = __uint128_t;
#else
    #error "128-bit integer support (__int128_t) is required for QykIs2 Engine."
#endif

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
    // TODO: 256要素のフラグ配列
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

#endif // QYKIS2_MATH_INTEGER_MATH_HPP