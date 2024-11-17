#include <chrono>
#include <iostream>
#include <map>

#include <execution>

#include "my_utils.h"
#include "data_loader.h"
#include "merge_sort.h"
#include "statistics.h"

constexpr std::string DATA_FILE = "ACC_001.csv";
const bool par = true; // parallel
const bool vec = true; // vectorized


int main() {
    // set number of threads - 1 for sequential, max for parallel
    set_num_threads(par);
    std::cout << "Running in " << (par ? "parallel" : "sequential") << " mode with " << (vec ? "vectorization" : "no vectorization") << std::endl;

    // load data from file
    struct data data;
    auto [load_time, load_ret] = measure_time(load_data, DATA_FILE, data);
    if (load_ret == EXIT_SUCCESS) {
        std::cout << "Data loaded in " << load_time << " seconds" << std::endl;
    } else {
        std::cerr << "Failed to load data" << std::endl;
    }

    for (const auto &[name, data_vec]: std::map<std::string, std::reference_wrapper<std::vector<double>>>{
            {"x", data.x},
            {"y", data.y},
            {"z", data.z}}) {
        std::cout << "\nColumn " << name << " :" << std::endl;

        size_t n = data_vec.get().size();

        std::cout << n << " elements" << std::endl;
        std::cout << "=============================" << std::endl;

        double CV = 0;
        double MAD = 0;
        auto [stat_time, stat_ret] = measure_time(compute_CV_MAD, data_vec, CV, MAD, vec);

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