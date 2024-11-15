#include "statistics.h"
#include "merge_sort.h"

int compute_CV_MAD(std::vector<double> &vec, double &cv, double &mad, const std::string& policy) {
    std::string policy_type = policy.substr(0, policy.find('_')), policy_vec = policy.substr(policy.find('_') + 1);
    if (policy_type == "par") { // parallel
        omp_set_num_threads(omp_get_max_threads());
    }
    else{ // sequential
        omp_set_num_threads(1);
    }

    double sum = 0;
    double sum2 = 0;
    size_t n = vec.size();
    auto [sort_time, sort_ret] = measure_time(mergeSort, vec, sum, sum2, "sequential");

    if (sort_ret == EXIT_SUCCESS && std::is_sorted(vec.begin(), vec.end())) {
        std::cout << "Sorted in " << sort_time << " seconds" << std::endl;
        cv = CV(sum, sum2, n);

        auto [mad_time, mad_ret] = measure_time(MAD, vec, n, "sequential");

        mad = mad_ret;
        return EXIT_SUCCESS;

    } else {
        std::cerr << "Failed to sort data" << std::endl;
        return EXIT_FAILURE;
    }
}


// coefficient of variance
double CV(double &sum, double &sum2, size_t n) {
    double mean = (double) sum / n;
    double variance = (double) sum2 / n - mean * mean;
    double cv = sqrt(variance) / mean;

    return cv;
}

double find_median(std::vector<double> &arr, size_t n) {
    size_t left_middle = (n - 1) / 2, right_middle = n / 2; // find the middle of the array
    // move from the middle to the beginning and end of the array n/2 times == median
    double prev = 0, curr = 0;
    for (size_t i = 0; i <= n / 2; i++) {
        prev = curr;
        curr = arr[left_middle] >= arr[right_middle] ? arr[right_middle++] : arr[left_middle--];
    }
    return n & 1 ? curr : (prev + curr) / 2.0;

}

// median absolute deviation
double MAD(std::vector<double> &arr, size_t n, std::string policy) {

    double median = (arr[n / 2] + arr[(n - 1) / 2]) / 2.0;
    // array of absolute differences from the median - size n
    std::vector<double> abs_diff(n);
    #pragma omp parallel for default(none) shared(arr, abs_diff, median, n)
    for (size_t i = 0; i < n; i++) {
        abs_diff[i] = std::abs(arr[i] - median);
    }

    return find_median(abs_diff, n);
}