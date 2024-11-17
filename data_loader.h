#pragma once

#include <iostream>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <omp.h>
#include <fstream>
#include <sstream>
#include <cstring>

/** 1 KB */
constexpr size_t KB = 1 << 10;
/** 1 MB */
constexpr size_t MB = 1 << 20;

/** Data structure to store accelerometer data */
struct data {
    std::vector<double> x;
    std::vector<double> y;
    std::vector<double> z;
};

/** Load accelerometer data from a file in serial
 * @param filename File to load data from
 * @param data Data structure to store the loaded data
 * @return EXIT_SUCCESS if the data was loaded successfully, EXIT_FAILURE otherwise
 */
int load_data_ser(const std::string &filename, data &data);

/** Load accelerometer data from a file in parallel
 * @param filename File to load data from
 * @param data Data structure to store the loaded data
 * @return EXIT_SUCCESS if the data was loaded successfully, EXIT_FAILURE otherwise
 */
int load_data_par(const std::string &filename, data &data);