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

void merge_no_count(std::vector<double> &arr, size_t l, size_t m, size_t r) {
    size_t n1 = m - l + 1; // size of left half
    size_t n2 = r - m; // size of right half

    // copy data to temp arrays L[] and R[]
    std::vector<double> L(arr.begin() + static_cast<int>(l),
                          arr.begin() + static_cast<int>(l) + static_cast<int>(n1));
    std::vector<double> R(arr.begin() + static_cast<int>(m) + 1,
                          arr.begin() + static_cast<int>(m) + 1 + static_cast<int>(n2));


    merge(arr, l, n1, n2, L, R);
}

void sum_and_copy(const std::vector<double> &arr, std::vector<double> &halve_arr,
                  size_t start, size_t size, double &sum, double &sum2) {

    size_t max_num_threads = std::thread::hardware_concurrency();
    size_t chunk_size = size / max_num_threads;

    // local sums and sum of squares for each thread
    std::vector<double> local_sums(max_num_threads, 0.0);
    std::vector<double> local_sums2(max_num_threads, 0.0);

    std::vector<size_t> chunk_indices(max_num_threads);
    std::iota(chunk_indices.begin(), chunk_indices.end(), 0); // pre-calculate chunk indices

    std::for_each(std::execution::par, chunk_indices.begin(), chunk_indices.end(),
                  [&](const auto &chunk_id) {
                      size_t start_chunk = chunk_id * chunk_size;
                      size_t end_chunk = (chunk_id == max_num_threads - 1) ? size : start_chunk + chunk_size;

                      for (size_t i = start_chunk; i < end_chunk; i++) {
                          double val = arr[i + start];  // get value from original array
                          halve_arr[i] = val;  // copy value to halve array

                          // accumulate sum and sum of squares for this chunk
                          local_sums[chunk_id] += val;
                          local_sums2[chunk_id] += val * val;
                      }
                  });

    // combine results from all threads - reduction of local sums
    sum += std::accumulate(local_sums.begin(), local_sums.end(), 0.0);
    sum2 += std::accumulate(local_sums2.begin(), local_sums2.end(), 0.0);
}

//void sum_and_copy_vec(const std::vector<double> &arr, std::vector<double> &halve,
//                      size_t start, size_t size, double &sum, double &sum2) {
//    size_t i, j;
//    size_t step = sizeof(__m256d) / sizeof(double);
//    size_t max_threads = omp_get_max_threads();
//    size_t chunk_size = size / max_threads;
//    chunk_size = chunk_size - chunk_size % step; // make sure the chunk size is divisible by the vector size
//
//    std::vector<double> local_sums(max_threads, 0);
//    std::vector<double> local_sums2(max_threads, 0);
//
//#pragma omp parallel default(none) shared(arr, halve, start, size, local_sums, local_sums2, chunk_size, step, max_threads) private(i, j)
//    for (i = 0; i < max_threads; i++) {
//        size_t start_chunk = start + i * chunk_size;
//        size_t end_chunk = start_chunk + chunk_size;
//
//        __m256d vec_sum = _mm256_setzero_pd();
//        __m256d vec_sum2 = _mm256_setzero_pd();
//
//        for (j = start_chunk; j < end_chunk; j += step) {
//            __m256d vec_vals = _mm256_loadu_pd(&arr[j]); // load 4 elements
//            _mm256_storeu_pd(&halve[j - start], vec_vals); // store 4 elements
//            vec_sum = _mm256_add_pd(vec_sum, vec_vals); // accumulate sum
//            vec_sum2 = _mm256_add_pd(vec_sum2, _mm256_mul_pd(vec_vals, vec_vals)); // accumulate sum of squares
//
//        }
//        // horizontal sum - sum of vector elements
//        std::vector<double> temp_sum(step);
//        std::vector<double> temp_sum2(step);
//        _mm256_storeu_pd(temp_sum.data(), vec_sum);
//        _mm256_storeu_pd(temp_sum2.data(), vec_sum2);
//        for (size_t k = 0; k < step; k++) {
//            local_sums[i] += temp_sum[k];
//            local_sums2[i] += temp_sum2[k];
//        }
//    }
//
//    // sum local sums
//    for (size_t k = 0; k < local_sums.size(); k++) {
//        sum += local_sums[k];
//        sum2 += local_sums2[k];
//    }
//
//    // handle remaining elements
//    for (i = chunk_size * max_threads; i < size; i++) {
//        double val = arr[start + i];
//        halve[i] = val;
//        sum += val;
//        sum2 += val * val;
//    }
//}

void sum_and_copy_vec(const std::vector<double> &arr, std::vector<double> &halve_arr,
                      size_t start, size_t size, double &sum, double &sum2) {

    size_t max_num_threads = std::thread::hardware_concurrency();
    size_t chunk_size = size / max_num_threads;
    size_t step = sizeof(__m256d) / sizeof(double);

    // Local sums and sum of squares for each thread
    std::vector<double> local_sums(max_num_threads, 0.0);
    std::vector<double> local_sums2(max_num_threads, 0.0);

    // Vector to hold the chunk indices for each thread
    std::vector<size_t> chunk_indices(max_num_threads);
    std::iota(chunk_indices.begin(), chunk_indices.end(), 0); // Pre-calculate chunk indices

    std::for_each(std::execution::par, chunk_indices.begin(), chunk_indices.end(),
                  [&](const auto &chunk_id) {
                      size_t start_chunk = chunk_id * chunk_size;
                      size_t end_chunk = (chunk_id == max_num_threads - 1) ? size : start_chunk + chunk_size;

                      __m256d vec_sum = _mm256_setzero_pd();
                      __m256d vec_sum2 = _mm256_setzero_pd();

                      // Process elements in chunks of 4 using AVX2
                      for (size_t i = start_chunk; i < end_chunk; i += step) {
                          __m256d vec_vals = _mm256_loadu_pd(&arr[i + start]); // load 4 elements
                          _mm256_storeu_pd(&halve_arr[i], vec_vals); // store 4 elements
                          vec_sum = _mm256_add_pd(vec_sum, vec_vals); // accumulate sum
                          vec_sum2 = _mm256_add_pd(vec_sum2, _mm256_mul_pd(vec_vals, vec_vals)); // accumulate sum of squares
                      }

                      // Horizontal sum - sum of vector elements
                      std::vector<double> temp_sum(step, 0.0);
                      std::vector<double> temp_sum2(step, 0.0);
                      _mm256_storeu_pd(temp_sum.data(), vec_sum);
                      _mm256_storeu_pd(temp_sum2.data(), vec_sum2);
                      for (size_t k = 0; k < step; k++) {
                          local_sums[chunk_id] += temp_sum[k];
                          local_sums2[chunk_id] += temp_sum2[k];
                      }

                      // Handle any remaining elements (less than 4) in the tail
                      for (size_t i = end_chunk - (end_chunk % step); i < end_chunk; i++) {
                          double val = arr[i + start];  // Get value from original array
                          halve_arr[i] = val;  // Copy value to halve array

                          // Accumulate sum and sum of squares for this chunk
                          local_sums[chunk_id] += val;
                          local_sums2[chunk_id] += val * val;
                      }
                  });

    // Combine results from all threads - reduction of local sums
    sum += std::accumulate(local_sums.begin(), local_sums.end(), 0.0);
    sum2 += std::accumulate(local_sums2.begin(), local_sums2.end(), 0.0);
}

void merge_and_count(std::vector<double> &arr, size_t l, size_t m, size_t r, double &sum, double &sum2,
                     const bool is_vectorized, const ExecutionPolicy &policy) {
    size_t n1 = m - l + 1;
    size_t n2 = r - m;

    std::vector<double> L(n1);
    std::vector<double> R(n2);

    if (is_vectorized) {
        sum_and_copy_vec(arr, L, l, n1, sum, sum2);
        sum_and_copy_vec(arr, R, m + 1, n2, sum, sum2);
    } else {
        sum_and_copy(arr, L, l, n1, sum, sum2);
        sum_and_copy(arr, R, m + 1, n2, sum, sum2);
    }

    merge(arr, l, n1, n2, L, R);
}


int mergeSort(std::vector<double> &arr, double &sum, double &sum2, const bool is_vectorized,
              const ExecutionPolicy &policy) {
    size_t n = arr.size();
    size_t curr_size;
    // divide the array into halves of size 1, 2, 4, 8, ... until the size is less than half the array size
    for (curr_size = 1; curr_size <= (n - 1) / 2; curr_size = 2 * curr_size) {
        // indices pre-calculation
        std::vector<size_t> left_starts;
        for (size_t left_start = 0; left_start < n - 1; left_start += 2 * curr_size) {
            left_starts.push_back(left_start);
        }
        // merge the halves
        std::visit([&](auto &&exec_policy) {
            std::for_each(exec_policy, left_starts.begin(), left_starts.end(), [&](size_t left_start) {
                size_t mid = std::min(left_start + curr_size - 1, n - 1);
                size_t right_end = std::min(left_start + 2 * curr_size - 1, n - 1);
                merge_no_count(arr, left_start, mid, right_end);
            });
        }, policy.get_policy());
    }
    // last iteration - merge and count the sum and sum of squares
    // indices pre-calculation
    std::vector<size_t> left_starts;
    for (size_t left_start = 0; left_start < n - 1; left_start += 2 * curr_size) {
        left_starts.push_back(left_start);
    }
    // merge the halves and count the sum and sum of squares
    std::visit([&](auto &&exec_policy) {
        std::for_each(exec_policy, left_starts.begin(), left_starts.end(), [&](size_t left_start) {
            size_t mid = std::min(left_start + curr_size - 1, n - 1);
            size_t right_end = std::min(left_start + 2 * curr_size - 1, n - 1);
            merge_and_count(arr, left_start, mid, right_end, sum, sum2, is_vectorized, policy);
        });
    }, policy.get_policy());

    return EXIT_SUCCESS;
}

