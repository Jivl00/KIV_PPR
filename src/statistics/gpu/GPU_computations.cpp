//
// Created by vladka on 22.11.2024.
//

#include "GPU_computations.h"

cl::Device GPU_computations::try_select_first_gpu() {
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

GPU_computations::GPU_computations(const char *kernelSource) {
    cl::Device device = try_select_first_gpu();

    std::vector<std::pair<const char *, size_t>> source_codes{{kernelSource, strlen(kernelSource)}};
    const cl::Program::Sources &sources(source_codes);
    context = cl::Context{device};
    program = cl::Program{context, sources};

//    program.build({device}, "-cl-std=CL2.0 -cl-denorms-are-zero");

    try {
        program.build({device});
        std::cerr << "Build Log:\n" << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
    } catch (const std::exception &e) {
        throw std::runtime_error("Failed to build OpenCL program");
    }

    queue = cl::CommandQueue{context, device};

}

void GPU_computations::abs_diff_calc(std::vector<double> &arr, std::vector<double> &abs_diff, double median, size_t n,
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

void GPU_computations::sum_vector(std::vector<double> &arr, double &sum, double &sum2, size_t n) {
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


void GPU_computations::sort_vector(std::vector<double> &arr, size_t n) {
    size_t padded_size = 1 << static_cast<size_t>(ceil(log2(n)));
    std::vector<double> padded_arr(padded_size, std::numeric_limits<double>::infinity());
    std::copy(arr.begin(), arr.end(), padded_arr.begin());

    // Temporary buffer for merging
    std::vector<double> temp(padded_size);

    cl::Buffer buffer_arr(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(double) * padded_size, padded_arr.data());
    cl::Buffer buffer_temp(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(double) * padded_size, temp.data());

    cl::Kernel kernel_merge_sort(program, "merge_sort");

    for (size_t width = 1; width < padded_size; width *= 2) {
        kernel_merge_sort.setArg(0, buffer_arr);
        kernel_merge_sort.setArg(1, buffer_temp);
        kernel_merge_sort.setArg(2, static_cast<unsigned int>(width));
        kernel_merge_sort.setArg(3, static_cast<unsigned int>(padded_size));

        cl::NDRange global(padded_size / (width * 2));
        queue.enqueueNDRangeKernel(kernel_merge_sort, cl::NullRange, global);
        queue.finish();
    }

    // Read the sorted array
    queue.enqueueReadBuffer(buffer_arr, CL_TRUE, 0, sizeof(double) * n, arr.data());
}


