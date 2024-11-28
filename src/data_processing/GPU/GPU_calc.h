#pragma once

#include <iostream>
#include <limits>
#include <vector>

#include <CL/cl.hpp>
#include "my_utils.h"
#include "statistics.h"

#ifdef _MSC_VER
#pragma comment(lib, "opencl.lib")
#endif

constexpr auto kernelSource = R"(
__kernel void abs_diff_calc(__global double *arr, __global double *abs_diff, double median) {
    int i = get_global_id(0);
    abs_diff[i] = fabs(arr[i] - median);
}

__kernel void vector_sums(
    __global const double* input,
    __global double* partialSums,
    __global double* partialSumsSquares,
    __local double* localSums,
    __local double* localSumsSquares) {

    uint global_id = get_global_id(0);
    uint local_id = get_local_id(0);
    uint group_size = get_local_size(0);
    uint group_id = get_group_id(0);

    // Initialize local memory
    if (global_id < get_global_size(0)) {
        double value = input[global_id];
        localSums[local_id] = value;
        localSumsSquares[local_id] = value * value;
    } else {
        localSums[local_id] = 0.0; // Handle out-of-bound threads
        localSumsSquares[local_id] = 0.0;
    }

    // Synchronize to ensure all work-items have written to local memory
    barrier(CLK_LOCAL_MEM_FENCE);

    // Reduction within the workgroup
    for (uint stride = group_size / 2; stride > 0; stride /= 2) {
        if (local_id < stride) {
            localSums[local_id] += localSums[local_id + stride];
            localSumsSquares[local_id] += localSumsSquares[local_id + stride];
        }
        barrier(CLK_LOCAL_MEM_FENCE); // Ensure updates are visible to all work-items
    }

    // Write the results of this workgroup to the partial sums arrays
    if (local_id == 0) {
        partialSums[group_id] = localSums[0];
        partialSumsSquares[group_id] = localSumsSquares[0];
    }
}

__kernel void merge_sort(__global double *arr, __global double *temp, const unsigned int width, const unsigned int size) {
    int global_id = get_global_id(0);
    int start = global_id * width * 2; // Starting index for this merge
    int mid = start + width;          // Middle index
    int end = min(start + 2 * width, size); // End index (clamped to size)

    if (mid >= size) {
        return; // Nothing to merge
    }

    int left = start;
    int right = mid;
    int index = start;

    // Merge two halves into the temp array
    while (left < mid && right < end) {
        if (arr[left] <= arr[right]) {
            temp[index++] = arr[left++];
        } else {
            temp[index++] = arr[right++];
        }
    }

    // Copy remaining elements from the left half
    while (left < mid) {
        temp[index++] = arr[left++];
    }

    // Copy remaining elements from the right half
    while (right < end) {
        temp[index++] = arr[right++];
    }

    // Copy sorted data back to the original array
    for (int i = start; i < end; i++) {
        arr[i] = temp[i];
    }
}
)";

/**
 * GPU_data_processing class used to compute the coefficient of variance and median absolute deviation.
 * Serves as a wrapper so std::visit can be used in the main function - so based on user input, the
 * appropriate computation can be used (CPU or GPU).
 * Basically, a static polymorphism is used here but without the ancestor class - not much in common
 * between the two classes.
 */
class GPU_data_processing {
public:
    static cl::Device try_select_first_gpu();
    explicit GPU_data_processing();
    void abs_diff_calc(std::vector<double> &arr, std::vector<double> &abs_diff, double median, size_t n,
                   bool is_vectorized, const execution_policy &policy);
    void sum_vector(std::vector<double> &arr, double &sum, double &sum2, size_t n);
    void sort_vector(std::vector<double> &arr, size_t n);
    /**
     * @brief Compute the coefficient of variance and median absolute deviation
     * @param vec - vector of doubles
     * @param cv - coefficient of variance (output)
     * @param mad - median absolute deviation (output)
     * @param is_vectorized - flag to indicate if vectorization is enabled
     * @param policy - execution policy - parallel or sequential
     * @return EXIT_SUCCESS if successful, EXIT_FAILURE otherwise
     */
    int compute_CV_MAD(std::vector<double> &vec, double &cv, double &mad, bool is_vectorized,
                       const execution_policy &policy);

private:
    cl::Context context;
    cl::CommandQueue queue;
    cl::Program program;
};
