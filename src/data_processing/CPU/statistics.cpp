#include "statistics.h"


double find_median(std::vector<double> &arr, size_t n) {
    size_t left_middle = (n - 1) / 2, right_middle = n / 2; // find the middle of the array
    // move from the middle to the beginning and end of the array n/2 times == median
    double prev = 0, curr = 0;
    for (size_t i = 0; i <= n / 2; i++) {
        prev = curr;
        curr = arr[left_middle] >= arr[right_middle] ? arr[right_middle++]
                                                     : arr[left_middle--]; // move to the next element
    }
    return n & 1 ? curr : (prev + curr) / 2.0; // if n is odd return the current element, else return the average
}


void abs_diff_calc(std::vector<double> &arr, std::vector<double> &abs_diff, double median, size_t n,
                   const bool is_vectorized, const execution_policy &policy) {
    size_t i = 0;
    if (is_vectorized) {

        // broadcast median to all elements of the vector
        __m256d med = _mm256_set1_pd(median);

        // create a mask with the sign bit cleared
        __m256d sign_mask = _mm256_set1_pd(-0.0);

        size_t step = sizeof(__m256d) / sizeof(double);

        std::vector<size_t> indices;
        for (i = 0; i <= n - step; i += step) {
            indices.push_back(i);
        }

        // process 4 elements at a time using AVX2
        std::visit([&](auto &&exec_policy) {
            std::for_each(exec_policy, indices.begin(), indices.end(), [&](size_t i) {
                __m256d vec = _mm256_loadu_pd(&arr[i]); // load 4 elements
                __m256d diff = _mm256_sub_pd(vec, med); // subtract median from elements
                __m256d abs_diff_vec = _mm256_andnot_pd(sign_mask, diff); // clear the sign bit
                _mm256_storeu_pd(&abs_diff[i], abs_diff_vec); // store result
            });
        }, policy.get_policy());
    }

    // process remaining elements
    int j = static_cast<int>(i);
    std::visit([&](auto &&exec_policy) {
        std::for_each(exec_policy, arr.begin() + j, arr.begin() + static_cast<int>(n), [&](double &value) {
            abs_diff[&value - &arr[0]] = std::abs(value - median);
        });
    }, policy.get_policy());
}


// coefficient of variance
double CV(double &sum, double &sum2, size_t n) {
    double mean = sum / (double) n; // calculate the mean
    double variance = sum2 / (double) n - mean * mean; // calculate the variance
    double cv = sqrt(variance) / mean;

    return cv;
}

// median absolute deviation
double MAD(std::vector<double> &arr, size_t n, const bool is_vectorized, const execution_policy &policy) {

    double median = (arr[n / 2] + arr[(n - 1) / 2]) / 2.0;
    // array of absolute differences from the median - size n
    std::vector<double> abs_diff_arr(n);
    abs_diff_calc(arr, abs_diff_arr, median, n, is_vectorized, policy);

    return find_median(abs_diff_arr, n);
}

int CPU_data_processing::compute_CV_MAD(std::vector<double> &vec, double &cv, double &mad, const bool is_vectorized,
                   const execution_policy &policy) {
    double sum = 0;
    double sum2 = 0;
    size_t n = vec.size();
    // sort the data
    auto [sort_time, sort_ret] = measure_time(mergeSort, vec, sum, sum2, is_vectorized, policy);

    // if sorting was successful calculate the coefficient of variance and median absolute deviation
    if (sort_ret == EXIT_SUCCESS && std::is_sorted(vec.begin(), vec.end())) {
        std::cout << "Sorted in " << sort_time << " seconds" << std::endl;
        cv = CV(sum, sum2, n);

        auto [mad_time, mad_ret] = measure_time(MAD, vec, n, is_vectorized, policy);

        mad = mad_ret;
        return EXIT_SUCCESS;

    } else {
        std::cerr << "Failed to sort data" << std::endl;
        return EXIT_FAILURE;
    }
}

