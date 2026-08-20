#ifndef QYKIS2_UTILS_STRING_UTILS_HPP
#define QYKIS2_UTILS_STRING_UTILS_HPP

#include "imgui.h"
#include <string>
#include <algorithm>
#include <gmp.h>
#include <gmpxx.h>
#include <stdexcept>
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
inline bool parse_rational_input(mpq_class& rop, const std::string& input_str) {
    if (input_str.empty()) return false;

    try {
        // "123" や "1/3" などの文字列から直接構築
        // 不正な文字列や "1/0" 等は例外が投げられる
        rop = mpq_class(input_str, 10);
        
        // 既約分数化（標準形化）
        rop.canonicalize();

        // 分母が 0 でないことの念のため確認
        if (rop.get_den() == 0) {
            return false;
        }

        return true;
    } catch (const std::invalid_argument&) {
        return false; // アルファベット混入やパース失敗
    } catch (const std::domain_error&) {
        return false; // ゼロ除算 (1/0 等)
    }
}

// std::string を ImGui::InputText で扱うためのヘルパー
static bool InputTextString(const char* label, std::string& str, ImGuiInputTextFlags flags = 0) {
    char buf[128];
    snprintf(buf, sizeof(buf), "%s", str.c_str());
    if (ImGui::InputText(label, buf, sizeof(buf), flags)) {
        str = buf;
        return true;
    }
    return false;
}

#endif // QYKIS2_UTILS_STRING_UTILS_HPP