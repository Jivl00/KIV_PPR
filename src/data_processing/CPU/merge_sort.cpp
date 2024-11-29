#include "merge_sort.h"


void sum_and_copy(const std::vector<real> &arr, std::vector<real> &halve_arr,
                  size_t start, size_t size, real &sum, real &sum2,
                  const execution_policy &policy) {

    size_t max_num_threads = std::thread::hardware_concurrency();
    size_t chunk_size = size / max_num_threads;

    // local sums and sum of squares for each thread
    std::vector<real> local_sums(max_num_threads, 0.0);
    std::vector<real> local_sums2(max_num_threads, 0.0);

    // vector to hold the chunk indices for each thread
    std::vector<size_t> chunk_indices(max_num_threads);
    std::iota(chunk_indices.begin(), chunk_indices.end(), 0); // pre-calculate chunk indices

    std::visit([&](auto &&exec_policy) {
        std::for_each(exec_policy, chunk_indices.begin(), chunk_indices.end(),
                      [&](const auto &chunk_id) {
                          size_t start_chunk = chunk_id * chunk_size;
                          size_t end_chunk = (chunk_id == max_num_threads - 1) ? size : start_chunk + chunk_size;

                          for (size_t i = start_chunk; i < end_chunk; i++) {
                              real val = arr[i + start];  // get value from original array
                              halve_arr[i] = val;  // copy value to halve array

                              // accumulate sum and sum of squares for this chunk
                              local_sums[chunk_id] += val;
                              local_sums2[chunk_id] += val * val;
                          }
                      });
    }, policy.get_policy());

    // combine results from all threads - reduction of local sums
    sum += std::accumulate(local_sums.begin(), local_sums.end(), 0.0);
    sum2 += std::accumulate(local_sums2.begin(), local_sums2.end(), 0.0);
}

void sum_and_copy_vec(const std::vector<real> &arr, std::vector<real> &halve_arr,
                      size_t start, size_t size, real &sum, real &sum2,
                      const execution_policy &policy) {

    size_t max_num_threads = std::thread::hardware_concurrency();
    size_t chunk_size = size / max_num_threads;
    size_t stride = sizeof(STRIDE) / sizeof(real);

    // local sums and sum of squares for each thread
    std::vector<real> local_sums(max_num_threads, 0.0);
    std::vector<real> local_sums2(max_num_threads, 0.0);

    // vector to hold the chunk indices for each thread
    std::vector<size_t> chunk_indices(max_num_threads);
    std::iota(chunk_indices.begin(), chunk_indices.end(), 0); // pre-calculate chunk indices

    std::visit([&](auto &&exec_policy) {
        std::for_each(exec_policy, chunk_indices.begin(), chunk_indices.end(),
                      [&](const auto &chunk_id) {
                          size_t start_chunk = chunk_id * chunk_size;
                          size_t end_chunk = (chunk_id == max_num_threads - 1) ? size : start_chunk + chunk_size;

                          auto vec_sum = SETZERO();
                          auto vec_sum2 = SETZERO();

                          for (size_t i = start_chunk; i < end_chunk; i += stride) {
                              auto vec_vals = LOAD(&arr[i + start]); // load elements
                              STORE(&halve_arr[i], vec_vals); // store elements
                              vec_sum = ADD(vec_sum, vec_vals); // accumulate sum
                              vec_sum2 = ADD(vec_sum2, MUL(vec_vals, vec_vals)); // accumulate sum of squares
                          }

                          // horizontal sum - sum of vector elements
                          std::vector<real> temp_sum(stride, 0.0);
                          std::vector<real> temp_sum2(stride, 0.0);
                          STORE(temp_sum.data(), vec_sum);
                          STORE(temp_sum2.data(), vec_sum2);
                          for (size_t k = 0; k < stride; k++) {
                              local_sums[chunk_id] += temp_sum[k];
                              local_sums2[chunk_id] += temp_sum2[k];
                          }

                          // handle any remaining elements (less than stride) in the tail
                          for (size_t i = end_chunk - (end_chunk % stride); i < end_chunk; i++) {
                              real val = arr[i + start];  // get value from original array
                              halve_arr[i] = val;  // copy value to halve array

                              // accumulate sum and sum of squares for this chunk
                              local_sums[chunk_id] += val;
                              local_sums2[chunk_id] += val * val;
                          }
                      });
    }, policy.get_policy());

    // combine results from all threads - reduction of local sums
    sum += std::accumulate(local_sums.begin(), local_sums.end(), 0.0);
    sum2 += std::accumulate(local_sums2.begin(), local_sums2.end(), 0.0);
}

void merge_and_count(std::vector<real> &arr, size_t l, size_t m, size_t r, real &sum, real &sum2,
                     const bool is_vectorized, const execution_policy &policy) {
    size_t n1 = m - l + 1;
    size_t n2 = r - m;

    std::vector<real> L(n1);
    std::vector<real> R(n2);

    if (is_vectorized) {
        sum_and_copy_vec(arr, L, l, n1, sum, sum2, policy);
        sum_and_copy_vec(arr, R, m + 1, n2, sum, sum2, policy);
    } else {
        sum_and_copy(arr, L, l, n1, sum, sum2, policy);
        sum_and_copy(arr, R, m + 1, n2, sum, sum2, policy);
    }

    merge(arr, l, n1, n2, L, R);
}

void merge_no_count(std::vector<real> &arr, size_t l, size_t m, size_t r) {
    size_t n1 = m - l + 1; // size of left half
    size_t n2 = r - m; // size of right half

    // copy data to temp arrays L[] and R[]
    std::vector<real> L(arr.begin() + static_cast<int>(l),
                          arr.begin() + static_cast<int>(l) + static_cast<int>(n1));
    std::vector<real> R(arr.begin() + static_cast<int>(m) + 1,
                          arr.begin() + static_cast<int>(m) + 1 + static_cast<int>(n2));


    merge(arr, l, n1, n2, L, R);
}

void merge(std::vector<real> &arr, size_t l, size_t n1, size_t n2, const std::vector<real> &L,
           const std::vector<real> &R) {
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

int mergeSort(std::vector<real> &arr, real &sum, real &sum2, const bool is_vectorized,
              const execution_policy &policy) {
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

