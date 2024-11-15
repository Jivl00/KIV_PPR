#pragma once

#include <cmath>
#include <vector>
#include <omp.h>

#include "my_utils.h"

int compute_CV_MAD(std::vector<double> &vec, double &cv, double &mad, const std::string& policy);
double CV(double &sum, double &sum2, size_t n);

double MAD(std::vector<double> &arr, size_t n, std::string policy);

double find_median(std::vector<double> &arr, size_t n);
