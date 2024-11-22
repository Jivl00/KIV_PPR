//#include "openCL_utils.h"
//#include <iostream>
//#include <vector>
//#include <numeric>
//#include <algorithm>
//
//const int median =  10;
//
//const char* kernelSource = R"(
//__kernel void vector_add_and_subtract(__global const double* A, __global const double* B, __global double* C, const int median) {
//    int id = get_global_id(0);
//    C[id] = fabs(A[id] + B[id] - median);
//}
//
//__kernel void sumGPU(__global const double* input, __global double* partialSums, __local double* localSums) {
//    uint local_id = get_local_id(0);
//    uint group_size = get_local_size(0);
//
//    // Copy from global to local memory
//    localSums[local_id] = input[get_global_id(0)];
//
//    // Loop for computing localSums: divide WorkGroup into 2 parts
//    for (uint stride = group_size / 2; stride > 0; stride /= 2) {
//        // Waiting for each 2x2 addition into given workgroup
//        barrier(CLK_LOCAL_MEM_FENCE);
//
//        // Add elements 2 by 2 between local_id and local_id + stride
//        if (local_id < stride) {
//            localSums[local_id] += localSums[local_id + stride];
//        }
//    }
//
//    // Write result into partialSums[nWorkGroups]
//    if (local_id == 0) {
//        partialSums[get_group_id(0)] = localSums[0];
//    }
//}
//)";
//
//int main() {
//    // Initialize data
//    const int elements = 1000000;
//    std::vector<double> A(elements, 5.0);
//    std::vector<double> B(elements, 3.0);
//    std::vector<double> C(elements);
//    double sum = 0.0;
//
//    // Setup OpenCL
//    cl::Context context;
//    cl::CommandQueue queue;
//    cl::Program program;
//    setupOpenCL(context, queue, program, kernelSource, "vector_add_and_subtract");
//
//    // Create buffers
//    cl::Buffer bufferA(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(double) * elements, A.data());
//    cl::Buffer bufferB(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(double) * elements, B.data());
//    cl::Buffer bufferC(context, CL_MEM_WRITE_ONLY, sizeof(double) * elements);
//    cl::Buffer bufferPartialSums(context, CL_MEM_READ_WRITE, sizeof(double) * (elements / 256));
//    cl::Buffer bufferSum(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(double), &sum);
//
//    // Check bufferA before kernel execution
//    std::vector<double> checkA(elements);
//    queue.enqueueReadBuffer(bufferA, CL_TRUE, 0, sizeof(double) * elements, checkA.data());
//
//    // Create kernel for vector_add_and_subtract
//    cl::Kernel kernel_add_subtract(program, "vector_add_and_subtract");
//
//    // Set kernel arguments for vector_add_and_subtract
//    kernel_add_subtract.setArg(0, bufferA);
//    kernel_add_subtract.setArg(1, bufferB);
//    kernel_add_subtract.setArg(2, bufferC);
//    kernel_add_subtract.setArg(3, median);
//
//    // Execute kernel for vector_add_and_subtract
//    cl::NDRange global(elements);
//    queue.enqueueNDRangeKernel(kernel_add_subtract, cl::NullRange, global, cl::NullRange);
//    queue.finish();
//
//    // Read results for vector_add_and_subtract
//    queue.enqueueReadBuffer(bufferC, CL_TRUE, 0, sizeof(double) * elements, C.data());
//
//    // Print results for vector_add_and_subtract
//    std::cout << "Results of vector_add_and_subtract:" << std::endl;
//    for (int i = 0; i < 10; ++i) {
//        std::cout << C[i] << " ";
//    }
//    std::cout << std::endl;
//
//    // Initialize sum to 0 before running the kernel
//    sum = 0.0;
//    queue.enqueueWriteBuffer(bufferSum, CL_TRUE, 0, sizeof(double), &sum);
//
//    std::cout << "First 10 elements of bufferA before kernel execution:" << std::endl;
//    for (int i = 0; i < 10; ++i) {
//        std::cout << checkA[i] << " ";
//    }
//    std::cout << std::endl;
//
//    // Create kernel for sumGPU
//    cl::Kernel kernel_sum(program, "sumGPU");
//
//    // Set kernel arguments for sumGPU
//    kernel_sum.setArg(0, bufferA);
//    kernel_sum.setArg(1, bufferPartialSums);
//    kernel_sum.setArg(2, cl::Local(sizeof(double) * 256)); // Assuming a local size of 256
//
//    // Execute kernel for sumGPU
//    cl::NDRange local(256);
//    queue.enqueueNDRangeKernel(kernel_sum, cl::NullRange, global, local);
//    queue.finish();
//
//    // Read partial sums
//    std::vector<double> partialSums(elements / 256);
//    queue.enqueueReadBuffer(bufferPartialSums, CL_TRUE, 0, sizeof(double) * (elements / 256), partialSums.data());
//
//    // Sum the partial sums on the host
//    sum = std::accumulate(partialSums.begin(), partialSums.end(), 0.0);
//
//    // Print results for sumGPU
//    std::cout << "Sum of elements in vector A: " << sum << std::endl;
//
//    return 0;
//}