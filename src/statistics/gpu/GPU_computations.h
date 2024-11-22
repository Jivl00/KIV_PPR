#pragma once

#include <iostream>
#include <limits>
#include <vector>

#include <CL/cl.hpp>
#include "my_utils.h"

#ifdef _MSC_VER
#pragma comment(lib, "opencl.lib")
#endif

class GPU_computations {
public:
    static cl::Device try_select_first_gpu();
    explicit GPU_computations(const char *kernelSource);
    void abs_diff_calc(std::vector<double> &arr, std::vector<double> &abs_diff, double median, size_t n,
                   bool is_vectorized, const ExecutionPolicy &policy);
    void sum_vector(std::vector<double> &arr, double &sum, double &sum2, size_t n);
    void sort_vector(std::vector<double> &arr, size_t n);

private:
    cl::Context context;
    cl::CommandQueue queue;
    cl::Program program;
};

