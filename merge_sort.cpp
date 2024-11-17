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


void
sum_and_copy(const std::vector<double> &arr, std::vector<double> &halve_arr, size_t start, size_t size, double &sum,
             double &sum2) {
    size_t i;

    std::vector<double> local_sums(omp_get_max_threads(), 0);
    std::vector<double> local_sums2(omp_get_max_threads(), 0);

#pragma omp parallel default(none) shared(arr, halve_arr, start, size, local_sums, local_sums2) private(i)
    {
        int thread_id = omp_get_thread_num();
        double local_sum = 0;
        double local_sum2 = 0;

#pragma omp for schedule(static)
        for (i = 0; i < size; i++) {
            double temp_val = arr[start + i];
            local_sum2 += temp_val * temp_val;
            local_sum += temp_val;
            halve_arr[i] = temp_val;
        }
        local_sums[thread_id] = local_sum;
        local_sums2[thread_id] = local_sum2;
    }

    for (i = 0; i < local_sums.size(); i++) {
        sum += local_sums[i];
        sum2 += local_sums2[i];
    }
}

void sum_and_copy_vec(const std::vector<double> &arr, std::vector<double> &halve,
                      size_t start, size_t size, double &sum, double &sum2) {
    size_t i;
    size_t step = sizeof(__m256d) / sizeof(double);

    __m256d vec_sum = _mm256_setzero_pd();
    __m256d vec_sum2 = _mm256_setzero_pd();

    for (i = 0; i < size - step; i += step) {
        __m256d vec_vals = _mm256_loadu_pd(&arr[start + i]); // load 4 elements
        _mm256_storeu_pd(&halve[i], vec_vals); // store 4 elements
        vec_sum = _mm256_add_pd(vec_sum, vec_vals); // accumulate sum
        vec_sum2 = _mm256_add_pd(vec_sum2, _mm256_mul_pd(vec_vals, vec_vals)); // accumulate sum of squares

    }

    // horizontal sum - sum of vector elements
    for (size_t j = 0; j < step; j++) {
        sum += vec_sum[j];
        sum2 += vec_sum2[j];
    }


    size_t j = i;
    // handle remaining elements
    for (i = j; i < size; i++) {
        double val = arr[start + i];
        halve[i] = val;
        sum += val;
        sum2 += val * val;
    }
}

void merge_and_count(std::vector<double> &arr, size_t l, size_t m, size_t r, double &sum, double &sum2,
                     const std::string &policy) {
    size_t n1 = m - l + 1;
    size_t n2 = r - m;

    std::vector<double> L(n1);
    std::vector<double> R(n2);

    sum_and_copy_vec(arr, L, l, n1, sum, sum2);
    sum_and_copy_vec(arr, R, m + 1, n2, sum, sum2);
//    sum_and_copy(arr, L, l, n1, sum, sum2);
//    sum_and_copy(arr, R, m + 1, n2, sum, sum2);

    merge(arr, l, n1, n2, L, R);
}


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

