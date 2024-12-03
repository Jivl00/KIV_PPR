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

#define WORK_GROUP_SIZE 256
#undef max

#include <limits>

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

        // initialize local memory
        float value = (global_id < get_global_size(0)) ? input[global_id] : 0.0;
        local_sums[local_id] = value;
        local_sums_squares[local_id] = value * value;

        // synchronize to ensure all work-items have written to local memory
        barrier(CLK_LOCAL_MEM_FENCE);

        // reduction within the workgroup
        for (uint stride = group_size / 2; stride > 0; stride /= 2) {
            if (local_id < stride) {
                local_sums[local_id] += local_sums[local_id + stride];
                local_sums_squares[local_id] += local_sums_squares[local_id + stride];
            }
            barrier(CLK_LOCAL_MEM_FENCE); // ensure updates are visible to all work-items
        }

        // write the results of this workgroup to the partial sums arrays
        if (local_id == 0) {
            partial_sums[group_id] = local_sums[0];
            partial_sums_squares[group_id] = local_sums_squares[0];
        }
    }

    __kernel void merge_sort(__global float *arr, __global float *temp, const unsigned int width, const unsigned int size) {
        int global_id = get_global_id(0);
        int start = global_id * width * 2; // starting index for this merge
        int mid = start + width;          // middle index
        int end = min(start + 2 * width, size); // end index (clamped to size)

        if (mid >= size) {
            return; // nothing to merge
        }

        int left = start;
        int right = mid;
        int index = start;

        // merge two halves into the temp array
        while (left < mid && right < end) {
            if (arr[left] <= arr[right]) {
                temp[index++] = arr[left++];
            } else {
                temp[index++] = arr[right++];
            }
        }

        // copy remaining elements from the left half
        while (left < mid) {
            temp[index++] = arr[left++];
        }

        // copy remaining elements from the right half
        while (right < end) {
            temp[index++] = arr[right++];
        }

        // copy sorted data back to the original array
        for (int i = start; i < end; i++) {
            arr[i] = temp[i];
        }
    }

    __kernel void bitonic_sort_kernel(__global float *arr, const uint stage, const uint pass_of_stage) {
        uint thread_id = get_global_id(0);
        uint pair_distance = 1 << (stage - pass_of_stage); // distance between elements to compare
        uint block_width = 2 * pair_distance; // width of the block being compared
        uint temp;
        bool compare_result;
        uint left_id = (thread_id & (pair_distance - 1)) + (thread_id >> (stage - pass_of_stage)) * block_width;
        uint right_id = left_id + pair_distance;

        // get the elements to compare
        float left_element = arr[left_id];
        float right_element = arr[right_id];

        uint same_direction_block_width = thread_id >> stage;
        uint same_direction = same_direction_block_width & 0x1; // check if the block is in the same direction

        // swap the elements if they are not in the same direction
        temp = same_direction ? right_id : temp;
        right_id = same_direction ? left_id : right_id;
        left_id = same_direction ? temp : left_id;

        compare_result = (left_element < right_element);

        // swap the elements if they are not in the correct order
        float greater = compare_result ? right_element : left_element;
        float lesser = compare_result ? left_element : right_element;

        arr[left_id] = lesser;
        arr[right_id] = greater;
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

        // initialize local memory
        double value = (global_id < get_global_size(0)) ? input[global_id] : 0.0;
        local_sums[local_id] = value;
        local_sums_squares[local_id] = value * value;

        // synchronize to ensure all work-items have written to local memory
        barrier(CLK_LOCAL_MEM_FENCE);

        // reduction within the workgroup
        for (uint stride = group_size / 2; stride > 0; stride /= 2) {
            if (local_id < stride) {
                local_sums[local_id] += local_sums[local_id + stride];
                local_sums_squares[local_id] += local_sums_squares[local_id + stride];
            }
            barrier(CLK_LOCAL_MEM_FENCE); // ensure updates are visible to all work-items
        }

        // write the results of this workgroup to the partial sums arrays
        if (local_id == 0) {
            partial_sums[group_id] = local_sums[0];
            partial_sums_squares[group_id] = local_sums_squares[0];
        }
    }

    __kernel void merge_sort(__global double *arr, __global double *temp, const unsigned int width, const unsigned int size) {
        int global_id = get_global_id(0);
        int start = global_id * width * 2; // starting index for this merge
        int mid = start + width;          // middle index
        int end = min(start + 2 * width, size); // end index (clamped to size)

        if (mid >= size) {
            return; // nothing to merge
        }

        int left = start;
        int right = mid;
        int index = start;

        // merge two halves into the temp array
        while (left < mid && right < end) {
            if (arr[left] <= arr[right]) {
                temp[index++] = arr[left++];
            } else {
                temp[index++] = arr[right++];
            }
        }

        // copy remaining elements from the left half
        while (left < mid) {
            temp[index++] = arr[left++];
        }

        // copy remaining elements from the right half
        while (right < end) {
            temp[index++] = arr[right++];
        }

        // copy sorted data back to the original array
        for (int i = start; i < end; i++) {
            arr[i] = temp[i];
        }
    }

    __kernel void bitonic_sort_kernel(__global double *arr, const uint stage, const uint pass_of_stage) {
        uint thread_id = get_global_id(0);
        uint pair_distance = 1 << (stage - pass_of_stage); // distance between elements to compare
        uint block_width = 2 * pair_distance; // width of the block being compared
        uint temp;
        bool compare_result;
        uint left_id = (thread_id & (pair_distance - 1)) + (thread_id >> (stage - pass_of_stage)) * block_width;
        uint right_id = left_id + pair_distance;

        // get the elements to compare
        double left_element = arr[left_id];
        double right_element = arr[right_id];

        uint same_direction_block_width = thread_id >> stage;
        uint same_direction = same_direction_block_width & 0x1; // check if the block is in the same direction

        // swap the elements if they are not in the same direction
        temp = same_direction ? right_id : temp;
        right_id = same_direction ? left_id : right_id;
        left_id = same_direction ? temp : left_id;

        compare_result = (left_element < right_element);

        // swap the elements if they are not in the correct order
        double greater = compare_result ? right_element : left_element;
        double lesser = compare_result ? left_element : right_element;

        arr[left_id] = lesser;
        arr[right_id] = greater;
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
    /**
     * @brief Constructor
     */
    explicit GPU_data_processing();

    /**
     * @brief Try to select the first GPU device available on the system
     * @return Device
     */
    static cl::Device try_select_first_gpu();

    /**
     * @brief Compute the sum and sum of squares of the vector
     * Premise: the GPU buffer is already set
     * @param sum - sum of the vector
     * @param sum2 - sum of squares of the vector
     * @param n - size of the vector without padding
     */
    void sum_vector(real &sum, real &sum2, size_t n);

    /**
     * @brief Sort the array using merge sort
     * @param arr - vector of reals
     * @param n - size of the vector without padding
     */
    void merge_sort(std::vector<real> &arr, size_t n);

    /**
     * @brief Sort the array using bitonic sort
     * Premise: the GPU buffer is already set
     * @param arr - vector of reals
     * @param n - size of the vector without padding
     */
    void bitonic_sort(std::vector<real> &arr, size_t n);

    /**
     * @brief Compute the absolute differences from the median
     * Premise: the GPU buffer is already set
     * @param abs_diff - vector of absolute differences
     * @param median - median value
     * @param n - size of the vector without padding
     */
    void abs_diff_calc(std::vector<real> &abs_diff, real median, size_t n);

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

    /**
     * @brief Set the buffer for the GPU
     * Pads the array to the nearest power of 2 and sets the buffer for the GPU
     * @param arr - vector of reals
     */
    void set_buffer(std::vector<real> &arr);

private:
    cl::Context context;
    cl::CommandQueue queue;
    cl::Program program;
    cl::Buffer padded_buffer_arr;
    size_t padded_size;
};
