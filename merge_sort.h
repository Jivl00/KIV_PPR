#pragma once

#include <vector>
#include <cmath>
#include <iostream>
#include <omp.h>
#include <algorithm>
#include <execution>
#include <immintrin.h>

#include "my_utils.h"

/**
 * @brief Standard merge function to merge two halves arr[l..m] and arr[m+1..r] - both halves are sorted
 * @param arr - vector to merge the halves to (output)
 * @param l - index to the arr where to store the merged halves
 * @param n1 - size of left half
 * @param n2 - size of right half
 * @param L - left half (size n1) - ascending order
 * @param R - right half (size n2) - ascending order
 */
void merge(std::vector<double> &arr, size_t l, size_t n1, size_t n2,
           const std::vector<double> &L, const std::vector<double> &R);

/**
 * @brief Merges two halves arr[l..m] and arr[m+1..r] of the array
 * @param arr - vector to merge
 * @param l - start index of the left half
 * @param m - middle index - end index of the left half and start index of the right half
 * @param r - end index of the right half
 */
void merge_no_count(std::vector<double> &arr, size_t l, size_t m, size_t r);

/**
 * @brief Merges two halves arr[l..m] and arr[m+1..r] of the array and counts the sum and sum of squared elements
 * @param arr - vector to merge
 * @param l - start index of the left half
 * @param m - middle index - end index of the left half and start index of the right half
 * @param r - end index of the right half
 * @param sum - sum of elements (output)
 * @param sum2 - sum of squared elements (output)
 * @param is_vectorized - flag to indicate if vectorization is enabled
 * @param policy - execution policy - parallel or sequential
 */
void merge_and_count(std::vector<double> &arr, size_t l, size_t m, size_t r,
                     double &sum, double &sum2, bool is_vectorized, const ExecutionPolicy &policy);

/**
 * @brief Merge sort algorithm to sort the vector and calculate sum and sum of squared elements
 * @param arr - vector to sort
 * @param sum - sum of elements (output)
 * @param sum2 - sum of squared elements (output)
 * @param is_vectorized - flag to indicate if vectorization is enabled
 * @param policy - execution policy - parallel or sequential
 * @return EXIT_SUCCESS if successful, EXIT_FAILURE otherwise
 */
int mergeSort(std::vector<double> &arr, double &sum, double &sum2, bool is_vectorized, const ExecutionPolicy &policy);
