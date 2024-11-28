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

class DeviceType {
public:
    enum class DType {
        CPU,
        GPU
    };
    const std::map<DType, std::variant<CPU_data_processing, GPU_data_processing>> device_map = {
            {DType::CPU, CPU_data_processing()},
            {DType::GPU, GPU_data_processing()}
    };

    explicit DeviceType(DType type) : type_(type) {}

    /**
     * @brief Get the device as a variant
     * @return std::variant<CPU_data_processing or GPU_data_processing>
     */
    [[nodiscard]] std::variant<CPU_data_processing, GPU_data_processing> get_device() const {
        return device_map.at(type_);
    }

private:
    DType type_;
};

