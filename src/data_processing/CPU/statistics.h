#pragma once

#include <vector>
#include <algorithm>
#include <execution>
#include <immintrin.h>

#include "my_utils.h"
#include "merge_sort.h"


/**
 * @brief Find the median of the bitonic array
 * @param arr - array of reals bitonically ordered
 * @param n - size of the array
 * @return median of the array
 */
real find_median(std::vector<real> &arr, size_t n);

/**
 * @brief Calculate the absolute difference of each element in the array from the median
 * @param arr - array of reals
 * @param abs_diff - array of absolute differences
 * @param median - median of the array  (output)
 * @param n - size of the array
 * @param is_vectorized - flag to indicate if vectorization is enabled
 * @param policy - execution policy - parallel or sequential
 */
void abs_diff_calc(std::vector<real> &arr, std::vector<real> &abs_diff, real median, size_t n,
                   bool is_vectorized, const execution_policy &policy);

/**
 * @brief Calculate the coefficient of variance
 * @param sum sum of the elements
 * @param sum2 sum of the squares of the elements
 * @param n size of the array
 * @return coefficient of variance
 */
real CV(real &sum, real &sum2, size_t n);

/**
 * @brief Calculate the median absolute deviation
 * @param arr - array of reals
 * @param n - size of the array
 * @param is_vectorized - flag to indicate if vectorization is enabled
 * @param policy - execution policy - parallel or sequential
 * @return median absolute deviation
 */
real MAD(std::vector<real> &arr, size_t n, bool is_vectorized, const execution_policy &policy);

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
     * @param vec - vector of reals
     * @param cv - coefficient of variance (output)
     * @param mad - median absolute deviation (output)
     * @param is_vectorized - flag to indicate if vectorization is enabled
     * @param policy - execution policy - parallel or sequential
     * @return EXIT_SUCCESS if successful, EXIT_FAILURE otherwise
     */
    static int compute_CV_MAD(std::vector<real> &vec, real &cv, real &mad, bool is_vectorized,
                       const execution_policy &policy);

};
