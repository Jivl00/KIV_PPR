#pragma once

#include <cmath>
#include <vector>
#include <omp.h>
#include <immintrin.h>

#include "my_utils.h"

int compute_CV_MAD(std::vector<double> &vec, double &cv, double &mad, bool policy);
double CV(double &sum, double &sum2, size_t n);

double MAD(std::vector<double> &arr, size_t n, bool policy);

double find_median(std::vector<double> &arr, size_t n);
