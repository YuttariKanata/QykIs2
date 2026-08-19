#ifndef QYKIS2_UTILS_STRING_UTILS_HPP
#define QYKIS2_UTILS_STRING_UTILS_HPP

#include <string>
#include <algorithm>
#include "engine/exact_eval.hpp" // int128_t の定義

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

#endif