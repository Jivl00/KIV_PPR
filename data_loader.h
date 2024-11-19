#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <execution>

/** Data structure to store accelerometer data */
struct data {
    std::vector<double> x;
    std::vector<double> y;
    std::vector<double> z;
};

/** Load accelerometer data from a file
 * @param filename File to load data from
 * @param data Data structure to store the loaded data
 * @return EXIT_SUCCESS if the data was loaded successfully, EXIT_FAILURE otherwise
 */
int load_data(const std::string &filename, data &data);