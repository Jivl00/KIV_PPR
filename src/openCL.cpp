#include "openCL_utils.h"
#include "GPU_computations.h"
#include <iostream>
#include <vector>
#include <limits>
#include <numeric>
#include <algorithm>


int main() {
    auto kernelSource = R"(
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
)";


    GPU_computations gpu_computations{kernelSource};

    std::vector<double> vec = {8, 6, 9, 5, -99, -6, 3, 44, 5, 2, 8, 888, -1, -22,55555};
//    std::vector<double> vec(20000000, 2.0);
//    std::iota(vec.begin(), vec.end(), 1);
    std::vector<double> abs_diff(vec.size());
    double median = 10.0;
    const bool par = true;
    ExecutionPolicy policy(par ? ExecutionPolicy::Type::Parallel : ExecutionPolicy::Type::Sequential);
    gpu_computations.abs_diff_calc(vec, abs_diff, median, vec.size(), false, policy);

    // print the first 10 elements of the result
    for (size_t i = 0; i < 10; i++) {
        std::cout << abs_diff[i] << " ";
    }

    // calculate the sum of the vector
    double sum = 0;
    double sum2 = 0;
    gpu_computations.sum_vector(vec, sum, sum2, vec.size());
    std::cout << "Sum: " << sum << std::endl;
    std::cout << "Sum of squares: " << sum2 << std::endl;

    std::cout << "check sum: " << std::accumulate(vec.begin(), vec.end(), 0.0) << std::endl;
    std::cout << "check sum of squares: " << std::inner_product(vec.begin(), vec.end(), vec.begin(), 0.0) << std::endl;


    return 0;
}