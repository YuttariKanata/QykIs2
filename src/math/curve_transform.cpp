#include "math/curve_transform.hpp"
#include "math/integer_math.hpp"
#include <vector>

bool normalize_curve(
    int degree,
    const mpq_class& in_coeff_y2,
    const mpq_class& in_coeff_x_max,
    const mpq_class& in_coeff_c2,
    const mpq_class& in_coeff_c3,
    const mpq_class& in_coeff_c4,
    const mpq_class& in_coeff_c5,
    const mpq_class& in_coeff_c6,
    StandardCurveConfig& out_config,
    CurveTransformInfo& out_transform
) {
    out_config.degree = degree;

    std::vector<mpq_class> raw_q = {
        in_coeff_y2,
        in_coeff_x_max,
        in_coeff_c2,
        in_coeff_c3,
        in_coeff_c4
    };
    if (degree >= 4) raw_q.push_back(in_coeff_c5);
    if (degree >= 5) raw_q.push_back(in_coeff_c6);

    // y^2 または最高次項の係数が 0 の場合は不正な入力
    if (raw_q[0] == 0 || raw_q[1] == 0) {
        return false;
    }

    // 1. 全項の分母の最小公倍数 L を求める
    mpz_class L = 1;
    for (const auto& q : raw_q) {
        L = lcm(L, q.get_den());
    }

    // 2. 両辺に L を掛けて全係数を整数化
    //    A y^2 = B x^d + C x^(d-1) + ...
    mpz_class A = raw_q[0].get_num() * (L / raw_q[0].get_den());

    std::vector<mpz_class> int_rhs;
    int_rhs.reserve(raw_q.size() - 1);
    for (size_t i = 1; i < raw_q.size(); ++i) {
        int_rhs.push_back(raw_q[i].get_num() * (L / raw_q[i].get_den()));
    }

    // 3. 両辺に A を掛けて Y = A * y に帰着
    //    Y^2 = A * (B x^d + C x^(d-1) + ...)
    //    逆変換パラメータ scale_A の格納 (int128_t 範囲内か検証)
    if (!mpz_class_to_int128(out_transform.scale_A, A)) {
        return false; // スケール倍率 A 自体が 128bit から溢れる場合
    }

    // 4. 右辺の各係数 (A * int_rhs[i]) を StandardCurveConfig に格納
    bool ok = true;
    ok &= mpz_class_to_int128(out_config.coeff_x_max, A * int_rhs[0]);
    ok &= mpz_class_to_int128(out_config.coeff_c2,    A * int_rhs[1]);
    ok &= mpz_class_to_int128(out_config.coeff_c3,    A * int_rhs[2]);
    ok &= mpz_class_to_int128(out_config.coeff_c4,    A * int_rhs[3]);

    if (degree >= 4) {
        ok &= mpz_class_to_int128(out_config.coeff_c5, A * int_rhs[4]);
    } else {
        out_config.coeff_c5 = 0;
    }

    if (degree >= 5) {
        ok &= mpz_class_to_int128(out_config.coeff_c6, A * int_rhs[5]);
    } else {
        out_config.coeff_c6 = 0;
    }

    return ok;
}

bool map_point_to_original(
    int128_t u,
    int128_t v,
    const CurveTransformInfo& transform,
    mpz_class& out_x,
    mpz_class& out_y
) {
    // 0 除算防止バリデーション
    if (transform.scale_A == 0) {
        return false;
    }

    // v が scale_A で割り切れない場合、元の曲線上で y が整解にならない
    if (v % transform.scale_A != 0) {
        return false;
    }

    // 1. x 座標の復元 (x = u)
    int128_to_mpz_class(out_x, u);

    // 2. y 座標の復元 (y = v / scale_A)
    int128_t y_val = v / transform.scale_A;
    int128_to_mpz_class(out_y, y_val);

    return true;
}