#pragma once

#include <variant>
#include <execution>
#include <map>
#include "statistics.h"
#include "GPU_calc.h"

/**
 * @brief Device type for computations
 *
 * @details
 *  - CPU
 *  - GPU
 */

class device_type {
public:
    enum class d_type {
        CPU,
        GPU
    };
    const std::map<d_type, std::variant<CPU_data_processing, GPU_data_processing>> device_map = {
            {d_type::CPU, CPU_data_processing()},
            {d_type::GPU, GPU_data_processing()}
    };

    explicit device_type(d_type type) : type_(type) {}

    /**
     * @brief Get the device as a variant
     * @return std::variant<CPU_data_processing or GPU_data_processing>
     */
    [[nodiscard]] std::variant<CPU_data_processing, GPU_data_processing> get_device() const {
        return device_map.at(type_);
    }

private:
    d_type type_;
};

