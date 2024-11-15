#pragma once

#include <vector>
#include <cmath>
#include <iostream>
#include <omp.h>

/**
 * Standard merge function to merge two halves arr[l..m] and arr[m+1..r]
 * @param arr - vector to merge
 * @param l - left index
 * @param n1 - size of left half
 * @param n2 - size of right half
 * @param L - left half
 * @param R - right half
 */
void merge(std::vector<double> &arr, size_t l, size_t n1, size_t n2,
           const std::vector<double> &L, const std::vector<double> &R);

/**
 * Standard merge function to merge two halves arr[l..m] and arr[m+1..r] without counting aditional statistics
 * @param arr - vector to merge
 * @param l - left index
 * @param m - middle index
 * @param r - right index
 * @param policy - execution policy
 */
void merge_no_count(std::vector<double> &arr, size_t l, size_t m, size_t r, const std::string& policy);

/**
 * Merge and count function for the last iteration of merge sort algorithm - counts sum and sum of squared elements
 * @param arr - vector to merge
 * @param l - left index
 * @param m - middle index
 * @param r - right index
 * @param sum - sum of elements
 * @param sum2 - sum of squared elements
 * @param policy - execution policy
 */
void merge_and_count(std::vector<double> &arr, size_t l, size_t m, size_t r,
                     double &sum, double &sum2, const std::string& policy);

/**
 * Merge sort algorithm
 * @param arr - vector to sort
 * @param sum - sum of elements
 * @param sum2 - sum of squared elements
 * @param policy - execution policy
 * @return 0 if successful, 1 otherwise
 */
int mergeSort(std::vector<double> &arr, double &sum, double &sum2, const std::string& policy);
