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

#ifdef _FLOAT
constexpr auto kernel_source = R"(
__kernel void abs_diff_calc(__global float *arr, float median) {
    int i = get_global_id(0);
    arr[i] = fabs(arr[i] - median);
}

__kernel void vector_sums(
    __global const float* input,
    __global float* partial_sums,
    __global float* partial_sums_squares,
    __local float* local_sums,
    __local float* local_sums_squares) {

    uint global_id = get_global_id(0);
    uint local_id = get_local_id(0);
    uint group_size = get_local_size(0);
    uint group_id = get_group_id(0);

    // Initialize local memory
    if (global_id < get_global_size(0)) {
        float value = input[global_id];
        local_sums[local_id] = value;
        local_sums_squares[local_id] = value * value;
    } else {
        local_sums[local_id] = 0.0f; // Handle out-of-bound threads
        local_sums_squares[local_id] = 0.0f;
    }

    // Synchronize to ensure all work-items have written to local memory
    barrier(CLK_LOCAL_MEM_FENCE);

    // Reduction within the workgroup
    for (uint stride = group_size / 2; stride > 0; stride /= 2) {
        if (local_id < stride) {
            local_sums[local_id] += local_sums[local_id + stride];
            local_sums_squares[local_id] += local_sums_squares[local_id + stride];
        }
        barrier(CLK_LOCAL_MEM_FENCE); // Ensure updates are visible to all work-items
    }

    // Write the results of this workgroup to the partial sums arrays
    if (local_id == 0) {
        partial_sums[group_id] = local_sums[0];
        partial_sums_squares[group_id] = local_sums_squares[0];
    }
}

__kernel void merge_sort(__global float *arr, __global float *temp, const unsigned int width, const unsigned int size) {
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
#else
constexpr auto kernel_source = R"(
__kernel void abs_diff_calc(__global double *arr, double median) {
    int i = get_global_id(0);
    arr[i] = fabs(arr[i] - median);
}

__kernel void vector_sums(
    __global const double* input,
    __global double* partial_sums,
    __global double* partial_sums_squares,
    __local double* local_sums,
    __local double* local_sums_squares) {

    uint global_id = get_global_id(0);
    uint local_id = get_local_id(0);
    uint group_size = get_local_size(0);
    uint group_id = get_group_id(0);

    // Initialize local memory
    if (global_id < get_global_size(0)) {
        double value = input[global_id];
        local_sums[local_id] = value;
        local_sums_squares[local_id] = value * value;
    } else {
        local_sums[local_id] = 0.0; // Handle out-of-bound threads
        local_sums_squares[local_id] = 0.0;
    }

    // Synchronize to ensure all work-items have written to local memory
    barrier(CLK_LOCAL_MEM_FENCE);

    // Reduction within the workgroup
    for (uint stride = group_size / 2; stride > 0; stride /= 2) {
        if (local_id < stride) {
            local_sums[local_id] += local_sums[local_id + stride];
            local_sums_squares[local_id] += local_sums_squares[local_id + stride];
        }
        barrier(CLK_LOCAL_MEM_FENCE); // Ensure updates are visible to all work-items
    }

    // Write the results of this workgroup to the partial sums arrays
    if (local_id == 0) {
        partial_sums[group_id] = local_sums[0];
        partial_sums_squares[group_id] = local_sums_squares[0];
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
#endif

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
    void abs_diff_calc(std::vector<real> &arr, std::vector<real> &abs_diff, real median, size_t n);
    void sum_vector(std::vector<real> &arr, real &sum, real &sum2, size_t n);
    void sort_vector(std::vector<real> &arr, size_t n);
    /**
     * @brief Compute the coefficient of variance and median absolute deviation
     * @param vec - vector of reals
     * @param cv - coefficient of variance (output)
     * @param mad - median absolute deviation (output)
     * @param is_vectorized - flag to indicate if vectorization is enabled
     * @param policy - execution policy - parallel or sequential
     * @return EXIT_SUCCESS if successful, EXIT_FAILURE otherwise
     */
    int compute_CV_MAD(std::vector<real> &vec, real &cv, real &mad, bool is_vectorized,
                       const execution_policy &policy);

private:
    cl::Context context;
    cl::CommandQueue queue;
    cl::Program program;
};
