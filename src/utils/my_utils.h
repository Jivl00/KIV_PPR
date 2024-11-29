#pragma once

#include <chrono>
#include <iostream>
#include <functional>
#include <tuple>
#include <variant>
#include <execution>
#include <map>
#include "data_processing/execution_policy.h"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifdef _FLOAT
using real = float;
#define LOAD _mm256_loadu_ps
#define STORE _mm256_storeu_ps
#define SETZERO _mm256_setzero_ps
#define ADD _mm256_add_ps
#define SUB _mm256_sub_ps
#define MUL _mm256_mul_ps
#define ANDNOT _mm256_andnot_ps
#define STRIDE __m256
#define SET1 _mm256_set1_ps
#define str_to_real std::strtof
#else
using real = double;
#define LOAD _mm256_loadu_pd
#define STORE _mm256_storeu_pd
#define SETZERO _mm256_setzero_pd
#define ADD _mm256_add_pd
#define SUB _mm256_sub_pd
#define MUL _mm256_mul_pd
#define ANDNOT _mm256_andnot_pd
#define STRIDE __m256d
#define SET1 _mm256_set1_pd
#define str_to_real std::strtod
#endif

/**
 * @brief Measure the time taken by a function to execute
 * Using variadic templates to accept any function and its arguments
 *
 * @tparam Func Function type
 * @tparam Args Argument types
 * @param f Function to measure
 * @param args Arguments to pass to the function
 * @return std::pair<double, decltype(f(args...))> Time taken and return value of the function
 */
template<typename Func, typename... Args>
auto measure_time(Func &&func, Args &&... args) {
    auto start = std::chrono::high_resolution_clock::now();
    auto result = std::forward<Func>(func)(std::forward<Args>(args)...);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    return std::make_tuple(duration.count(), result);
}
