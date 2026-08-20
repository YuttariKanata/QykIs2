#ifndef QYKIS2_MATH_CURVE_EVAL_HPP
#define QYKIS2_MATH_CURVE_EVAL_HPP

#include "math/curve_transform.hpp"
#include "math/integer_math.hpp"
#include <optional>

// --------------------------------------------------
// 3次 / 4次 / 5次曲線の Horner 法厳密評価関数
// 各次数のウェイト斉次化に対応した評価式
// --------------------------------------------------

// 3次: Y^2 = c1*X^3 + c2*X^2*d^2 + c3*X*d^4 + c4*d^6 (x = X/d^2, y = Y/d^3)
// Horner形: ((c1*X + c2*d^2)*X + c3*d^4)*X + c4*d^6
inline std::optional<int128_t> eval_exact_deg3(
    const StandardCurveConfig& config,
    int64_t X_in,
    int64_t d_in
) {
    const int128_t X = X_in;
    const int128_t d = d_in;

    const int128_t c1 = config.coeff_x_max;
    const int128_t c2 = config.coeff_c2;
    const int128_t c3 = config.coeff_c3;
    const int128_t c4 = config.coeff_c4;

    const int128_t d2 = d * d;
    const int128_t d4 = d2 * d2;
    const int128_t d6 = d4 * d2;

    const int128_t T = ((c1 * X + c2 * d2) * X + c3 * d4) * X + c4 * d6;

    return check_perfect_square(T);
}

// 4次: Y^2 = c1*X^4 + c2*X^3*d + c3*X^2*d^2 + c4*X*d^3 + c5*d^4 (x = X/d, y = Y/d^2)
// Horner形: (((c1*X + c2*d)*X + c3*d^2)*X + c4*d^3)*X + c5*d^4
inline std::optional<int128_t> eval_exact_deg4(
    const StandardCurveConfig& config,
    int64_t X_in,
    int64_t d_in
) {
    const int128_t X = X_in;
    const int128_t d = d_in;

    const int128_t c1 = config.coeff_x_max;
    const int128_t c2 = config.coeff_c2;
    const int128_t c3 = config.coeff_c3;
    const int128_t c4 = config.coeff_c4;
    const int128_t c5 = config.coeff_c5;

    const int128_t d2 = d * d;
    const int128_t d3 = d2 * d;
    const int128_t d4 = d2 * d2;

    const int128_t T = (((c1 * X + c2 * d) * X + c3 * d2) * X + c4 * d3) * X + c5 * d4;

    return check_perfect_square(T);
}

// 5次: Y^2 = c1*X^5 + c2*X^4*d^2 + c3*X^3*d^4 + c4*X^2*d^6 + c5*X*d^8 + c6*d^10 (x = X/d^2, y = Y/d^5)
// Horner形: ((((c1*X + c2*d^2)*X + c3*d^4)*X + c4*d^6)*X + c5*d^8)*X + c6*d^10
inline std::optional<int128_t> eval_exact_deg5(
    const StandardCurveConfig& config,
    int64_t X_in,
    int64_t d_in
) {
    const int128_t X = X_in;
    const int128_t d = d_in;

    const int128_t c1 = config.coeff_x_max;
    const int128_t c2 = config.coeff_c2;
    const int128_t c3 = config.coeff_c3;
    const int128_t c4 = config.coeff_c4;
    const int128_t c5 = config.coeff_c5;
    const int128_t c6 = config.coeff_c6;

    const int128_t d2  = d * d;
    const int128_t d4  = d2 * d2;
    const int128_t d6  = d4 * d2;
    const int128_t d8  = d4 * d4;
    const int128_t d10 = d8 * d2;

    const int128_t T = ((((c1 * X + c2 * d2) * X + c3 * d4) * X + c4 * d6) * X + c5 * d8) * X + c6 * d10;

    return check_perfect_square(T);
}

#endif // QYKIS2_MATH_CURVE_EVAL_HPP