#pragma once

#include <tuple>
#include <variant>
#include <execution>
#include <functional>
#include <map>


/**
 * @brief Execution policy for parallelism
 *
 * @details
 *  - Sequential: std::execution::seq
 *  - Parallel: std::execution::par
 */
class execution_policy {
public:
    enum class e_type {
        Sequential,
        Parallel
    };
    const std::map<e_type, std::variant<std::execution::sequenced_policy, std::execution::parallel_policy>> policy_map = {
            {e_type::Sequential, std::execution::seq},
            {e_type::Parallel,   std::execution::par}
    };

    explicit execution_policy(e_type type) : type_(type) {}

    /**
     * @brief Get the execution policy as a variant
     * @return std::variant<std::execution::sequenced_policy or std::execution::parallel_policy>
     */
    [[nodiscard]] std::variant<std::execution::sequenced_policy, std::execution::parallel_policy> get_policy() const {
        return policy_map.at(type_);
    }

private:
    e_type type_;
};