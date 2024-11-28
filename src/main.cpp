#include <chrono>
#include <iostream>
#include <map>
#include <execution>

#include "my_utils.h"
#include "data_loader.h"
#include "my_utils2.h"

const std::string DATA_FILE = "data/ACC_001.csv";
const bool par = true; // parallel
const bool vec = true; // vectorized
const bool gpu = false; // GPU


int main() {
    // set device type - CPU or GPU
    DeviceType device(gpu ? DeviceType::DType::GPU : DeviceType::DType::CPU);
    std::cout << "Running on " << (gpu ? "GPU" : "CPU") << std::endl;
    // set execution policy - parallel or sequential
    ExecutionPolicy policy(par ? ExecutionPolicy::Type::Parallel : ExecutionPolicy::Type::Sequential);
    std::cout << "Running in " << (par ? "parallel" : "sequential") << " mode with " << (vec ? "vectorization" : "no vectorization") << std::endl;

    // load data from file
    struct data data;
    auto [load_time, load_ret] = measure_time([&](const std::string& filename, struct data& data, const ExecutionPolicy& policy) {
        return load_data(filename, data, policy);
    }, DATA_FILE, data, std::cref(policy));

    if (load_ret == EXIT_SUCCESS) {
        std::cout << "Data loaded in " << load_time << " seconds" << std::endl;
    } else {
        std::cerr << "Failed to load data" << std::endl;
    }

    std::map<std::string, std::reference_wrapper<std::vector<double>>> data_map = {
            {"x", data.x},
            {"y", data.y},
            {"z", data.z}
    };

    for (const auto& pair : data_map) {
        const std::string& name = pair.first;
        std::vector<double>& data_vec = pair.second;

        std::cout << "\nColumn " << name << " :" << std::endl;

        size_t n = data_vec.size();

        std::cout << n << " elements" << std::endl;
        std::cout << "=============================" << std::endl;

        double CV = 0;
        double MAD = 0;

        std::visit([&](auto &&device) {
            auto [stat_time, stat_ret] = measure_time([&](std::vector<double> &vec, double &cv, double &mad, bool is_vectorized, const ExecutionPolicy &policy) {
                return device.compute_CV_MAD(vec, cv, mad, is_vectorized, policy);
            }, data_vec, CV, MAD, vec, std::cref(policy));

            if (stat_ret == EXIT_SUCCESS) {
                std::cout << "Coefficient of variance: " << CV << std::endl;
                std::cout << "Median absolute deviation: " << MAD << std::endl;
                std::cout << "Computed in " << stat_time << " seconds" << std::endl;
            } else {
                std::cerr << "Failed to compute statistics" << std::endl;
            }

        }, device.get_device());

    }

    return 0;
}