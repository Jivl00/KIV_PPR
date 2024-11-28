#include "GPU_calc.h"

cl::Device GPU_data_processing::try_select_first_gpu() {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    for (auto &platform: platforms) {
        std::vector<cl::Device> devices;
        platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);

        for (auto &device: devices) {
            auto desc = device.getInfo<CL_DEVICE_NAME>();
            std::cout << "Selected device " << desc << " on platform " << platform.getInfo<CL_PLATFORM_NAME>()
                      << std::endl;
            return device;
        }
    }

    cl::Platform platform = cl::Platform::getDefault();
    cl::Device device = cl::Device::getDefault();
    auto desc = device.getInfo<CL_DEVICE_NAME>();
    std::cout << "Selected default device " << desc << " on platform " << platform.getInfo<CL_PLATFORM_NAME>()
              << std::endl;
    return device;
}

GPU_data_processing::GPU_data_processing() {
    cl::Device device = try_select_first_gpu();

    std::vector<std::pair<const char *, size_t>> source_codes{{kernelSource, strlen(kernelSource)}};
    const cl::Program::Sources &sources(source_codes);
    context = cl::Context{device};
    program = cl::Program{context, sources};

//    program.build({device}, "-cl-std=CL2.0 -cl-denorms-are-zero");

    /* Try to build it */
    try {
        program.build({device});
        if (program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(device) != CL_BUILD_SUCCESS)
            std::cerr << "Build Log:\n" << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Build Log:\n" << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
        throw std::runtime_error("Failed to build OpenCL program");
    }

    queue = cl::CommandQueue{context, device};

}

void GPU_data_processing::abs_diff_calc(std::vector<double> &arr, std::vector<double> &abs_diff, double median, size_t n,
                                        bool is_vectorized, const ExecutionPolicy &policy) {
    // Create buffers
    cl::Buffer buffer_arr(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(double) * n, arr.data());
    cl::Buffer buffer_abs_diff(context, CL_MEM_WRITE_ONLY, sizeof(double) * n);

    // Create kernel for abs_diff_calc
    cl::Kernel kernel_abs_diff(program, "abs_diff_calc");

    // Set kernel arguments for abs_diff_calc
    kernel_abs_diff.setArg(0, buffer_arr);
    kernel_abs_diff.setArg(1, buffer_abs_diff);
    kernel_abs_diff.setArg(2, median);

    // Execute kernel for abs_diff_calc
    cl::NDRange global(n);
    queue.enqueueNDRangeKernel(kernel_abs_diff, cl::NullRange, global);
    queue.finish();

    // Read results for abs_diff_calc
    queue.enqueueReadBuffer(buffer_abs_diff, CL_TRUE, 0, sizeof(double) * n, abs_diff.data());
}

void GPU_data_processing::sum_vector(std::vector<double> &arr, double &sum, double &sum2, size_t n) {
    const size_t workgroup_size = 256; // Choose a workgroup size
    const size_t global_size = ((n + workgroup_size - 1) / workgroup_size) * workgroup_size;
    const size_t num_workgroups = global_size / workgroup_size;

    // Create buffers
    cl::Buffer buffer_arr(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(double) * n, arr.data());
    cl::Buffer buffer_partial_sums(context, CL_MEM_WRITE_ONLY, sizeof(double) * num_workgroups);
    cl::Buffer buffer_partial_sums_squares(context, CL_MEM_WRITE_ONLY, sizeof(double) * num_workgroups);

    // Create kernel for vector_sum
    cl::Kernel kernel_vector_sum(program, "vector_sums");

    // Set kernel arguments for vector_sum
    kernel_vector_sum.setArg(0, buffer_arr);
    kernel_vector_sum.setArg(1, buffer_partial_sums);
    kernel_vector_sum.setArg(2, buffer_partial_sums_squares);
    kernel_vector_sum.setArg(3, cl::Local(sizeof(double) * workgroup_size));
    kernel_vector_sum.setArg(4, cl::Local(sizeof(double) * workgroup_size));

    // Execute kernel for vector_sum
    cl::NDRange global(global_size);
    cl::NDRange local(workgroup_size);
    queue.enqueueNDRangeKernel(kernel_vector_sum, cl::NullRange, global, local);
    queue.finish();

    // Read partial sums from device
    std::vector<double> partial_sums(num_workgroups);
    std::vector<double> partial_sums_squares(num_workgroups);
    queue.enqueueReadBuffer(buffer_partial_sums, CL_TRUE, 0, sizeof(double) * num_workgroups, partial_sums.data());
    queue.enqueueReadBuffer(buffer_partial_sums_squares, CL_TRUE, 0, sizeof(double) * num_workgroups, partial_sums_squares.data());

    // Final reduction on the host
    sum = std::accumulate(partial_sums.begin(), partial_sums.end(), 0.0);
    sum2 = std::accumulate(partial_sums_squares.begin(), partial_sums_squares.end(), 0.0);
}


void GPU_data_processing::sort_vector(std::vector<double> &arr, size_t n) {
    // Temporary buffer for merging
    std::vector<double> temp(n);

    cl::Buffer buffer_arr(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(double) * n, arr.data());
    cl::Buffer buffer_temp(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(double) * n, temp.data());

    cl::Kernel kernel_merge_sort(program, "merge_sort");

    for (size_t width = 1; width < n; width *= 2) {
        kernel_merge_sort.setArg(0, buffer_arr);
        kernel_merge_sort.setArg(1, buffer_temp);
        kernel_merge_sort.setArg(2, static_cast<unsigned int>(width));
        kernel_merge_sort.setArg(3, static_cast<unsigned int>(n));

        cl::NDRange global(n / (width * 2));
        queue.enqueueNDRangeKernel(kernel_merge_sort, cl::NullRange, global);
        queue.finish();
    }

    // Read the sorted array
    queue.enqueueReadBuffer(buffer_arr, CL_TRUE, 0, sizeof(double) * n, arr.data());
}

int GPU_data_processing::compute_CV_MAD(std::vector<double> &vec, double &cv, double &mad, bool is_vectorized,
                                        const ExecutionPolicy &policy) {
    double sum = 0;
    double sum2 = 0;
    size_t n = vec.size();
    // sort the data

//    auto [sort_time, sort_ret] = measure_time(mergeSort, vec, sum, sum2, is_vectorized, policy);

    // if sorting was successful calculate the coefficient of variance and median absolute deviation
//    if (sort_ret == EXIT_SUCCESS && std::is_sorted(vec.begin(), vec.end())) {
//        std::cout << "Sorted in " << sort_time << " seconds" << std::endl;
//        cv = CV(sum, sum2, n);

//        auto [mad_time, mad_ret] = measure_time(MAD, vec, n, is_vectorized, policy);

//        mad = mad_ret;
//        return EXIT_SUCCESS;
//
//    } else {
//        std::cerr << "Failed to sort data" << std::endl;
//        return EXIT_FAILURE;
//    }

    this->sum_vector(vec, sum, sum2, n);

    this->sort_vector(vec, n);
    auto median = (vec[n / 2] + vec[(n - 1) / 2]) / 2.0;
    this->abs_diff_calc(vec, vec, median, n, is_vectorized, policy);
    this->sort_vector(vec, n);
    mad = (vec[n / 2] + vec[(n - 1) / 2]) / 2.0;

    double mean = sum / (double) n; // calculate the mean
    double variance = sum2 / (double) n - mean * mean; // calculate the variance
    cv = sqrt(variance) / mean;

    return EXIT_SUCCESS;
}
