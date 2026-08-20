#ifndef QYKIS2_UTILS_STRING_UTILS_HPP
#define QYKIS2_UTILS_STRING_UTILS_HPP

#include <string>
#include <algorithm>
#include <gmp.h>
#include "math/curve_eval.hpp" // int128_t の定義

// --------------------------------------------------
// 128bit 整数を十進数文字列へ変換
// --------------------------------------------------
inline std::string to_string_128(int128_t val) {
    if (val == 0) return "0";
    bool is_neg = (val < 0);
    int128_t abs_val = is_neg ? -val : val;

    std::string str;
    while (abs_val > 0) {
        char digit = static_cast<char>(abs_val % 10);
        str += ('0' + digit);
        abs_val /= 10;
    }
    if (is_neg) str += '-';
    std::reverse(str.begin(), str.end());
    return str;
}

// --------------------------------------------------
// mpq_set_str による有理数文字列のパース & バリデーション
// --------------------------------------------------
inline bool parse_rational_input(mpq_t rop, const std::string& input_str) {
    if (input_str.empty()) return false;

    // 整数 "123" や 分数 "1/3", " 1 / 3 " をそのまま読み込める
    if (mpq_set_str(rop, input_str.c_str(), 10) != 0) {
        return false; // 不正文字列（アルファベット混入、記号重複など）
    }

    // 分母が 0 判定 ("1/0" や "0/0" 対策)
    if (mpz_cmp_ui(mpq_denref(rop), 0) == 0) {
        return false;
    }

    // 既約分数化（標準形化）
    mpq_canonicalize(rop);

    return true;
}

#endif // QYKIS2_UTILS_STRING_UTILS_HPP