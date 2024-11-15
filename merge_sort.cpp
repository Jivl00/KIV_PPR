#include "merge_sort.h"


void merge(std::vector<double> &arr, size_t l, size_t n1, size_t n2, const std::vector<double> &L,
           const std::vector<double> &R) {
    size_t i = 0, j = 0, k = l;
    while (i < n1 && j < n2) { // while there are elements in both halves
        arr[k++] = L[i] <= R[j] ? L[i++] : R[j++]; // copy the smaller element
    }

    // copy the remaining elements, if there are any
    while (i < n1) {
        arr[k++] = L[i++];
    }

    while (j < n2) {
        arr[k++] = R[j++];
    }
}

void merge_no_count(std::vector<double> &arr, size_t l, size_t m, size_t r, const std::string &policy) {
    size_t n1 = m - l + 1; // size of left half
    size_t n2 = r - m; // size of right half

    // copy data to temp arrays L[] and R[]
    std::vector<double> L(arr.begin() + static_cast<int>(l),
                          arr.begin() + static_cast<int>(l) + static_cast<int>(n1));
    std::vector<double> R(arr.begin() + static_cast<int>(m) + 1,
                          arr.begin() + static_cast<int>(m) + 1 + static_cast<int>(n2));


    merge(arr, l, n1, n2, L, R);
}

void merge_and_count(std::vector<double> &arr, size_t l, size_t m, size_t r,
                     double &sum, double &sum2, const std::string &policy) {
    size_t n1 = m - l + 1; // size of left half
    size_t n2 = r - m; // size of right half

    std::vector<double> L(n1);
    std::vector<double> R(n2);

    // copy data to temp arrays L[] and R[]
    std::vector<double> local_sums(omp_get_max_threads(), 0);
    std::vector<double> local_sums2(omp_get_max_threads(), 0);

    // parallel region
#pragma omp parallel default(none) shared(arr, L, R, l, m, n1, n2, sum, sum2, local_sums, local_sums2) num_threads(omp_get_max_threads())
    {
        // local variables for each thread
        int thread_id = omp_get_thread_num();
        double local_sum = 0;
        double local_sum2 = 0;
        size_t i;

#pragma omp for schedule(static) // static scheduling - each thread gets a chunk of iterations
        for (i = 0; i < n1; i++) {
            double temp = arr[l + i];
            local_sum2 += temp * temp;
            local_sum += temp;
            L[i] = temp;
        }
        local_sums[thread_id] = local_sum;
        local_sums2[thread_id] = local_sum2;
    }
    // sum the local sums and local sums of squared elements
    for (size_t i = 0; i < local_sums.size(); i++) {
        sum += local_sums[i];
        sum2 += local_sums2[i];
    }

#pragma omp parallel default(none) shared(arr, L, R, l, m, n1, n2, sum, sum2, local_sums, local_sums2) num_threads(omp_get_max_threads())
    {
        // local variables for each thread
        int thread_id = omp_get_thread_num();
        double local_sum = 0;
        double local_sum2 = 0;
        size_t j;

#pragma omp for schedule(static)
        for (j = 0; j < n2; j++) {
            double temp = arr[m + 1 + j];
            local_sum2 += temp * temp;
            local_sum += temp;
            R[j] = temp;
        }
        local_sums[thread_id] = local_sum;
        local_sums2[thread_id] = local_sum2;
    }
    // sum the local sums and local sums of squared elements
    for (size_t i = 0; i < local_sums.size(); i++) {
        sum += local_sums[i];
        sum2 += local_sums2[i];
    }

    merge(arr, l, n1, n2, L, R);

}


// Iterative merge_no_count sort function
int mergeSort(std::vector<double> &arr, double &sum, double &sum2, const std::string &policy) {
    size_t n = arr.size();
    size_t curr_size;
    for (curr_size = 1; curr_size <= (n - 1) / 2; curr_size = 2 * curr_size) {
        size_t left_start;
#pragma omp parallel for default(none) shared(arr, n, curr_size, policy) private(left_start)
        for (left_start = 0; left_start < n - 1; left_start += 2 * curr_size) {
            size_t mid = std::min(left_start + curr_size - 1, n - 1);
            size_t right_end = std::min(left_start + 2 * curr_size - 1, n - 1);
            merge_no_count(arr, left_start, mid, right_end, policy);
        }
    }
    size_t left_start;
#pragma omp parallel for default(none) shared(arr, n, curr_size, sum, sum2, policy) private(left_start)
    for (left_start = 0; left_start < n - 1; left_start += 2 * curr_size) {
        size_t mid = std::min(left_start + curr_size - 1, n - 1);
        size_t right_end = std::min(left_start + 2 * curr_size - 1, n - 1);
        merge_and_count(arr, left_start, mid, right_end, sum, sum2, policy);
    }
    return EXIT_SUCCESS;
}

