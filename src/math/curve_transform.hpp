#ifndef MATH_CURVE_TRANSFORM_HPP
#define MATH_CURVE_TRANSFORM_HPP

#include <gmpxx.h>
#include <cstdint>

// 128-bit 符号付き整数の型定義 (GCC / Clang 拡張)
using int128_t = __int128_t;

// --------------------------------------------------
// 変換後の標準形設定 (Y^2 = f(x))
// 探索エンジン (Engine) および ModSieve へ直接渡す構造体
// --------------------------------------------------
struct StandardCurveConfig {
    int degree = 3; // 曲線次数 (3, 4, 5)

    int128_t coeff_x_max = 0; 
    int128_t coeff_c2    = 0; 
    int128_t coeff_c3    = 0; 
    int128_t coeff_c4    = 0; 
    int128_t coeff_c5    = 0; 
    int128_t coeff_c6    = 0; 
};

// --------------------------------------------------
// 逆変換情報構造体
// (Y^2 = A * f(x) の A のみを保持するPOD)
// --------------------------------------------------
struct CurveTransformInfo {
    // Y = A * y のスケール倍率 A (解の復元時: y = Y / scale_A)
    int128_t scale_A = 1;
};

// --------------------------------------------------
// 関数の宣言
// --------------------------------------------------

/**
 * @brief 有理数係数の一般形 y^2 式から、整係数標準形 Y^2 = A * f(x) への正規化を行う
 * 
 * @param degree 次数 (3, 4, 5)
 * @param in_coeff_y2    y^2 の係数 (a)
 * @param in_coeff_x_max x^d の係数 (b)
 * @param in_coeff_c2    x^(d-1) の係数 (c)
 * @param in_coeff_c3    x^(d-2) の係数 (d)
 * @param in_coeff_c4    x^(d-3) の係数 (e)
 * @param in_coeff_c5    x^(d-4) の係数 (f) (4次以上)
 * @param in_coeff_c6    定数項       (g) (5次以上)
 * @param out_config     [out] 変換後の標準形パラメータ
 * @param out_transform  [out] 逆変換用情報 (scale_A)
 * @return bool 係数が int128_t に収まり正常終了したか
 */
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
);

/**
 * @brief 探索によって見つかった標準形上の点 (u, v) を元の曲線上の (x, y) に復元する
 * 
 * @param u 探索結果の X 座標 (x = u)
 * @param v 探索結果の Y 座標 (Y = v)
 * @param transform 逆変換情報
 * @param out_x [out] 復元された元の x 座標 (mpz_class)
 * @param out_y [out] 復元された元の y 座標 (mpz_class)
 * @return bool v % scale_A == 0 であり、元の曲線上で整点yとなったか
 */
bool map_point_to_original(
    int128_t u,
    int128_t v,
    const CurveTransformInfo& transform,
    mpz_class& out_x,
    mpz_class& out_y
);

#endif // MATH_CURVE_TRANSFORM_HPP