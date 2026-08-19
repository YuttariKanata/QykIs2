#ifndef QYKIS2_ENGINE_EXACT_EVAL_HPP
#define QYKIS2_ENGINE_EXACT_EVAL_HPP

#include <cmath>
#include <cstdint>
#include <optional>

// --------------------------------------------------
// 128 bit 整数の型定義 (GCC/Clang 環境前提)
// --------------------------------------------------
#if defined(__SIZEOF_INT128__)
    using int128_t = __int128_t;
    using uint128_t = __uint128_t;
#else
    #error "128-bit integer support (__int128_t) is required for QykIs2 Engine."
#endif

// --------------------------------------------------
// ビット操作・超高速 128bit isqrt モジュール
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

// Bit-Shift & Single-Division による超高速 128bit isqrt
inline uint128_t isqrt128(uint128_t n) {
    if (n <= 1) return n;

    int lz = clz128(n);
    if (lz >= 64) return isqrt64(static_cast<uint64_t>(n));

    int a = (lz == 2) ? 1 : (lz - 1) / 2;

    uint128_t scaled_n = n << (2 * a);
    uint64_t u = static_cast<uint64_t>(scaled_n >> 64);
    uint64_t isqu = isqrt64(u);

    uint128_t x = ((static_cast<uint128_t>(isqu) << 32) + 
                  ((((u - isqu * isqu) << 31) | (static_cast<uint64_t>(scaled_n) >> 33)) / isqu)) >> a;

    while (x < UINT64_MAX && (x + 1) * (x + 1) <= n) x++;
    while (x > 0 && (x > UINT64_MAX || x * x > n)) x--;
    return x;
}

// --------------------------------------------------
// Mod 256 プリフィルター & 完全平方数判定
// --------------------------------------------------

// Mod 256 の 256 byte ルックアップテーブル
// (256 パターンのうち、平方余剰となる 44 パターンのみ 1, 他は 0)
alignas(64) static const uint8_t SQ_MOD256_LOOKUP[256] = {
    // TODO: ここに 256 要素のフラグ配列 (0 or 1) を埋める
};

inline std::optional<int128_t> check_perfect_square(int128_t n) {
    if (n < 0) return std::nullopt;
    if (n == 0) return 0;

    // 1. Mod 256 プリフィルター (82.8% 撃墜)
    const uint8_t mod256 = static_cast<uint8_t>(static_cast<uint64_t>(n) & 0xFF);
    if (!SQ_MOD256_LOOKUP[mod256]) {
        return std::nullopt;
    }

    // 2. 最速 isqrt128 による判定
    const uint128_t un = static_cast<uint128_t>(n);
    const uint128_t r = isqrt128(un);

    if (r * r == un) {
        return static_cast<int128_t>(r);
    }

    return std::nullopt;
}

// --------------------------------------------------
// 3次 / 4次曲線の厳密評価関数
// --------------------------------------------------

// 3次曲線: T = X^3 + a*d^4*X + b*d^6 の評価
inline std::optional<int128_t> eval_exact_deg3(int64_t a, int64_t b, int64_t X_in, int64_t d_in) {
    const int128_t X = X_in;
    const int128_t d = d_in;
    const int128_t ma = a;
    const int128_t mb = b;

    const int128_t d2 = d * d;
    const int128_t d4 = d2 * d2;
    const int128_t d6 = d4 * d2;

    const int128_t T = X * X * X + ma * d4 * X + mb * d6;

    return check_perfect_square(T);
}

// 4次曲線: T = X^4 + a*X^2*d^2 + b*X*d^3 + c*d^4 の評価
inline std::optional<int128_t> eval_exact_deg4(int64_t a, int64_t b, int64_t c, int64_t X_in, int64_t d_in) {
    const int128_t X = X_in;
    const int128_t d = d_in;
    const int128_t ma = a;
    const int128_t mb = b;
    const int128_t mc = c;

    const int128_t X2 = X * X;
    const int128_t X4 = X2 * X2;

    const int128_t d2 = d * d;
    const int128_t d3 = d2 * d;
    const int128_t d4 = d2 * d2;

    const int128_t T = X4 + ma * X2 * d2 + mb * X * d3 + mc * d4;

    return check_perfect_square(T);
}

#endif // QYKIS2_ENGINE_EXACT_EVAL_HPP