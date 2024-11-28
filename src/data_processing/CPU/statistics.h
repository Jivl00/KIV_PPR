#pragma once

#include <vector>
#include <algorithm>
#include <execution>
#include <immintrin.h>

#include "my_utils.h"
#include "merge_sort.h"


/**
 * @brief Find the median of the bitonic array
 * @param arr - array of doubles bitonically ordered
 * @param n - size of the array
 * @return median of the array
 */
double find_median(std::vector<double> &arr, size_t n);

/**
 * @brief Calculate the absolute difference of each element in the array from the median
 * @param arr - array of doubles
 * @param abs_diff - array of absolute differences
 * @param median - median of the array  (output)
 * @param n - size of the array
 * @param is_vectorized - flag to indicate if vectorization is enabled
 * @param policy - execution policy - parallel or sequential
 */
void abs_diff_calc(std::vector<double> &arr, std::vector<double> &abs_diff, double median, size_t n,
                   bool is_vectorized, const execution_policy &policy);

/**
 * @brief Calculate the coefficient of variance
 * @param sum sum of the elements
 * @param sum2 sum of the squares of the elements
 * @param n size of the array
 * @return coefficient of variance
 */
double CV(double &sum, double &sum2, size_t n);

/**
 * @brief Calculate the median absolute deviation
 * @param arr - array of doubles
 * @param n - size of the array
 * @param is_vectorized - flag to indicate if vectorization is enabled
 * @param policy - execution policy - parallel or sequential
 * @return median absolute deviation
 */
double MAD(std::vector<double> &arr, size_t n, bool is_vectorized, const execution_policy &policy);

/**
 * CPU_data_processing class used to compute the coefficient of variance and median absolute deviation.
 * Serves as a wrapper so std::visit can be used in the main function - so based on user input, the
 * appropriate computation can be used (CPU or GPU).
 * Basically, a static polymorphism is used here but without the ancestor class - not much in common
 * between the two classes.
 */
class CPU_data_processing {
public:

    /**
     * @brief Compute the coefficient of variance and median absolute deviation
     * @param vec - vector of doubles
     * @param cv - coefficient of variance (output)
     * @param mad - median absolute deviation (output)
     * @param is_vectorized - flag to indicate if vectorization is enabled
     * @param policy - execution policy - parallel or sequential
     * @return EXIT_SUCCESS if successful, EXIT_FAILURE otherwise
     */
    int compute_CV_MAD(std::vector<double> &vec, double &cv, double &mad, bool is_vectorized,
                       const execution_policy &policy);

};
