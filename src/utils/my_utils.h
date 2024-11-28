#pragma once

#include <chrono>
#include <iostream>
#include <functional>
#include <tuple>
#include <variant>
#include <execution>
#include <map>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
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


/**
 * @brief Execution policy for parallelism
 *
 * @details
 *  - Sequential: std::execution::seq
 *  - Parallel: std::execution::par
 */
class ExecutionPolicy {
public:
    enum class Type {
        Sequential,
        Parallel
    };
    const std::map<Type, std::variant<std::execution::sequenced_policy, std::execution::parallel_policy>> policy_map = {
            {Type::Sequential, std::execution::seq},
            {Type::Parallel,   std::execution::par}
    };

    explicit ExecutionPolicy(Type type) : type_(type) {}

    /**
     * @brief Get the execution policy as a variant
     * @return std::variant<std::execution::sequenced_policy or std::execution::parallel_policy>
     */
    [[nodiscard]] std::variant<std::execution::sequenced_policy, std::execution::parallel_policy> get_policy() const {
        return policy_map.at(type_);
    }

private:
    Type type_;
};
