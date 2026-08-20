#ifndef QYKIS2_MATH_CURVE_EVAL_HPP
#define QYKIS2_MATH_CURVE_EVAL_HPP

#include "math/integer_math.hpp"

// --------------------------------------------------
// 3次 / 4次 / 5次曲線の Horner 法厳密評価関数
// --------------------------------------------------

// 3次: T = X^3 + (a*X + b*d^2)*d^4
inline std::optional<int128_t> eval_exact_deg3(int64_t a, int64_t b, int64_t X_in, int64_t d_in) {
    const int128_t X = X_in;
    const int128_t d = d_in;
    const int128_t ma = a;
    const int128_t mb = b;

    const int128_t d2 = d * d;
    const int128_t d4 = d2 * d2;

    const int128_t T = X * X * X + (ma * X + mb * d2) * d4;

    return check_perfect_square(T);
}

// 4次: T = X^4 + ((c*d + b*X)*d + a*X^2)*d^2
inline std::optional<int128_t> eval_exact_deg4(int64_t a, int64_t b, int64_t c, int64_t X_in, int64_t d_in) {
    const int128_t X = X_in;
    const int128_t d = d_in;
    const int128_t ma = a;
    const int128_t mb = b;
    const int128_t mc = c;

    const int128_t X2 = X * X;
    const int128_t X4 = X2 * X2;
    const int128_t d2 = d * d;

    const int128_t T = ((mc * d + mb * X) * d + ma * X2) * d2 + X4;

    return check_perfect_square(T);
}

// 5次: T = X^5 + (((d*de^2 + c*X)*de^2 + b*X^2)*de^2 + a*X^3)*de^4
inline std::optional<int128_t> eval_exact_deg5(int64_t a, int64_t b, int64_t c, int64_t d_coeff, int64_t X_in, int64_t de_in) {
    const int128_t X = X_in;
    const int128_t de = de_in;
    const int128_t ma = a;
    const int128_t mb = b;
    const int128_t mc = c;
    const int128_t md = d_coeff;

    const int128_t X2 = X * X;
    const int128_t X3 = X2 * X;
    const int128_t X5 = X3 * X2;

    const int128_t de2 = de * de;
    const int128_t de4 = de2 * de2;

    const int128_t T = (((md * de2 + mc * X) * de2 + mb * X2) * de2 + ma * X3) * de4 + X5;

    return check_perfect_square(T);
}

#endif // QYKIS2_MATH_CURVE_EVAL_HPP