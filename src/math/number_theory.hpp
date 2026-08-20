#ifndef QYKIS2_MATH_NUMBER_THEORY_HPP
#define QYKIS2_MATH_NUMBER_THEORY_HPP

#include <vector>

// エラトステネスの篩
inline std::vector<int> generate_primes(int limit) {
    if (limit < 2) return {};
    std::vector<bool> is_prime(limit + 1, true);
    is_prime[0] = is_prime[1] = false;
    for (int p = 2; p * p <= limit; ++p) {
        if (is_prime[p]) {
            for (int i = p * p; i <= limit; i += p) {
                is_prime[i] = false;
            }
        }
    }

    std::vector<int> primes;
    for (int p = 2; p <= limit; ++p) {
        if (is_prime[p]) primes.push_back(p);
    }
    return primes;
}

#endif // QYKIS2_MATH_NUMBER_THEORY_HPP