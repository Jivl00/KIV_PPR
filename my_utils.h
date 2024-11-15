#pragma once

#include <chrono>
#include <iostream>
#include <functional>
#include <tuple>

/**
 * @brief Measure the time taken by a function to execute
 *
 * @tparam Func Function type
 * @tparam Args Argument types
 * @param f Function to measure
 * @param args Arguments to pass to the function
 * @return std::pair<double, decltype(f(args...))> Time taken and return value of the function
 */
template<typename Func, typename... Args>
auto measure_time(Func f, Args&&... args) {
    auto start = std::chrono::high_resolution_clock::now();
    auto result = std::invoke(f, std::forward<Args>(args)...);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    return std::make_pair(elapsed.count(), result);
}
