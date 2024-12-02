#include "statistics.h"


real find_median(std::vector<real> &arr, size_t n) {
    size_t left_middle = (n - 1) / 2, right_middle = n / 2; // find the middle of the array
    // move from the middle to the beginning and end of the array n/2 times == median
    real prev = 0, curr = 0;
    for (size_t i = 0; i <= n / 2; i++) {
        prev = curr;
        curr = arr[left_middle] >= arr[right_middle] ? arr[right_middle++]
                                                     : arr[left_middle--]; // move to the next element
    }
    return n & 1 ? curr : (prev + curr) / static_cast<real>(2.0); // if n is odd return the current element, else return the average
}


void abs_diff_calc(std::vector<real> &arr, std::vector<real> &abs_diff, real median, size_t n,
                   const bool is_vectorized, const execution_policy &policy) {
    size_t i = 0;
    if (is_vectorized) {

        // broadcast median to all elements of the vector
        auto med = SET1(median);

        // create a mask with the sign bit cleared
        auto sign_mask = SET1(-0.0);

        size_t step = sizeof(STRIDE) / sizeof(real);

        std::vector<size_t> indices;
        for (i = 0; i <= n - step; i += step) {
            indices.push_back(i);
        }

        // process 4 elements at a time using AVX2
        std::visit([&](auto &&exec_policy) {
            std::for_each(exec_policy, indices.begin(), indices.end(), [&](size_t i) {
                auto vec = LOAD(&arr[i]); // load 4 elements
                auto diff = SUB(vec, med); // subtract median from elements
                auto abs_diff_vec = ANDNOT(sign_mask, diff); // clear the sign bit
                STORE(&abs_diff[i], abs_diff_vec); // store result
            });
        }, policy.get_policy());
    }

    // process remaining elements
    int j = static_cast<int>(i);
    std::visit([&](auto &&exec_policy) {
        std::for_each(exec_policy, arr.begin() + j, arr.begin() + static_cast<int>(n), [&](real &value) {
            abs_diff[&value - &arr[0]] = std::abs(value - median);
        });
    }, policy.get_policy());
}


// coefficient of variance
real CV(real &sum, real &sum2, size_t n) {
    real mean = sum / (real) n; // calculate the mean
    real variance = sum2 / (real) n - mean * mean; // calculate the variance
    real cv = sqrt(variance) / mean;

    return cv;
}

// median absolute deviation
real MAD(std::vector<real> &arr, size_t n, const bool is_vectorized, const execution_policy &policy) {

    real median = (arr[n / 2] + arr[(n - 1) / 2]) / static_cast<real>(2.0);

    // compute array of absolute differences from the median
    abs_diff_calc(arr, arr, median, n, is_vectorized, policy);

    return find_median(arr, n);
}

int CPU_data_processing::compute_CV_MAD(std::vector<real> &vec, real &cv, real &mad, const bool is_vectorized,
                   const execution_policy &policy) {
    real sum = 0;
    real sum2 = 0;
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

