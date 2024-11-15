#include <chrono>
#include <iostream>
#include <map>

#include <execution>

#include "my_utils.h"
#include "data_loader.h"
#include "merge_sort.h"
#include "statistics.h"

constexpr std::string DATA_FILE = "ACC_001.csv";

// Load data from file ACC_001.csv
int main() {
    // Load data from file
    struct data data;
    auto [load_time, load_ret] = measure_time(load_data_par, DATA_FILE, data);
    if (load_ret == EXIT_SUCCESS) {
        std::cout << "Data loaded in " << load_time << " seconds" << std::endl;
    } else {
        std::cerr << "Failed to load data" << std::endl;
    }

    // Sort data and calculate statistics for each column - x, y, z
    for (const auto &[name, vec]: std::map<std::string, std::reference_wrapper<std::vector<double>>>{
            {"x", data.x},
            {"y", data.y},
            {"z", data.z}}) {
        std::cout << "\nColumn " << name << " :" << std::endl;

        size_t n = vec.get().size();

        std::cout << n << " elements" << std::endl;
        std::cout << "=============================" << std::endl;

        double CV = 0;
        double MAD = 0;
        auto [stat_time, stat_ret] = measure_time(compute_CV_MAD, vec, CV, MAD, "par_vec");

        if (stat_ret == EXIT_SUCCESS) {
            std::cout << "Coefficient of variance: " << CV << std::endl;
            std::cout << "Median absolute deviation: " << MAD << std::endl;
            std::cout << "Computed in " << stat_time << " seconds" << std::endl;
        } else {
            std::cerr << "Failed to compute statistics" << std::endl;
        }

    }

    return 0;
}