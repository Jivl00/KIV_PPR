#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <execution>
#include "my_utils.h"
#include "data_processing/device_type.h"

/** Data structure to store accelerometer data */
struct data {
    std::vector<double> x;
    std::vector<double> y;
    std::vector<double> z;
};

/**
 * @brief Load accelerometer data from a file
 * @param filename File to load data from
 * @param data Data structure to store the loaded data
 * @param policy Execution policy
 * @return EXIT_SUCCESS if the data was loaded successfully, EXIT_FAILURE otherwise
 */
int load_data(const std::string &filename, data &data, const execution_policy &policy);