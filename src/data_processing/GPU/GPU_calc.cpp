#include "GPU_calc.h"

cl::Device GPU_data_processing::try_select_first_gpu() {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    for (auto &platform: platforms) {
        std::vector<cl::Device> devices;
        platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);

        if (!devices.empty()) {
            auto &device = devices.front();
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

void GPU_data_processing::set_buffer(std::vector<real> &arr) {
    size_t n = arr.size();
    // pad the input to the nearest power of 2
    size_t power = 1;
    while (power < n) {
        power <<= 1;
    }
    arr.resize(power, std::numeric_limits<real>::max());
    padded_size = power;

    padded_buffer_arr =cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(real) * power, arr.data());

}

GPU_data_processing::GPU_data_processing() : context(), program(), queue(), padded_size() {
    cl::Device device = try_select_first_gpu();

    std::vector<std::pair<const char *, size_t>> source_codes{{kernel_source, strlen(kernel_source)}};
    const cl::Program::Sources &sources(source_codes);
    context = cl::Context{device};
    program = cl::Program{context, sources};

    try {
        program.build({device});
        if (program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(device) != CL_BUILD_SUCCESS)
            std::cerr << "Build Log:\n" << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Build Log:\n" << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
        std::cerr << e.what() << std::endl;
        throw std::runtime_error("Failed to build OpenCL program");
    }

    queue = cl::CommandQueue{context, device};

}

void GPU_data_processing::abs_diff_calc(std::vector<real> &abs_diff, real median, size_t n) {

    // create sub-buffer - array without the padding
    cl_buffer_region region = {0, sizeof(real) * n};
    cl::Buffer sub_buffer = padded_buffer_arr.createSubBuffer(CL_MEM_READ_ONLY, CL_BUFFER_CREATE_TYPE_REGION, &region);

    // create kernel
    cl::Kernel kernel_abs_diff(program, "abs_diff_calc");

    // set kernel arguments
    kernel_abs_diff.setArg(0, sub_buffer);
    kernel_abs_diff.setArg(1, median);

    // execute kernel
    cl::NDRange global(n);
    queue.enqueueNDRangeKernel(kernel_abs_diff, cl::NullRange, global);
    queue.finish();

    // read the result back to the host
    queue.enqueueReadBuffer(sub_buffer, CL_TRUE, 0, sizeof(real) * n, abs_diff.data());
}

void GPU_data_processing::sum_vector(real &sum, real &sum2, size_t n) {

    // calculate the global size and number of workgroups
    const size_t global_size = ((n + WORK_GROUP_SIZE - 1) / WORK_GROUP_SIZE) * WORK_GROUP_SIZE;
    const size_t num_workgroups = global_size / WORK_GROUP_SIZE;

    // create buffers for partial sums and partial sums of squares
    cl::Buffer buffer_partial_sums(context, CL_MEM_WRITE_ONLY, sizeof(real) * num_workgroups);
    cl::Buffer buffer_partial_sums_squares(context, CL_MEM_WRITE_ONLY, sizeof(real) * num_workgroups);

    // create sub-buffer - array without the padding
    cl_buffer_region region = {0, sizeof(real) * n};
    cl::Buffer sub_buffer = padded_buffer_arr.createSubBuffer(CL_MEM_READ_ONLY, CL_BUFFER_CREATE_TYPE_REGION, &region);

    // create kernel
    cl::Kernel kernel_vector_sum(program, "vector_sums");

    // set kernel arguments
    kernel_vector_sum.setArg(0, sub_buffer);
    kernel_vector_sum.setArg(1, buffer_partial_sums);
    kernel_vector_sum.setArg(2, buffer_partial_sums_squares);
    kernel_vector_sum.setArg(3, cl::Local(sizeof(real) * WORK_GROUP_SIZE)); // local memory for partial sums
    kernel_vector_sum.setArg(4, cl::Local(sizeof(real) * WORK_GROUP_SIZE)); // local memory for partial sums of squares

    // execute kernel
    cl::NDRange global(global_size);
    cl::NDRange local(WORK_GROUP_SIZE);
    queue.enqueueNDRangeKernel(kernel_vector_sum, cl::NullRange, global, local);
    queue.finish();

    // read the partial sums back to the host
    std::vector<real> partial_sums(num_workgroups);
    std::vector<real> partial_sums_squares(num_workgroups);
    queue.enqueueReadBuffer(buffer_partial_sums, CL_TRUE, 0, sizeof(real) * num_workgroups, partial_sums.data());
    queue.enqueueReadBuffer(buffer_partial_sums_squares, CL_TRUE, 0, sizeof(real) * num_workgroups, partial_sums_squares.data());

    // calculate the total sum and sum of squares - reduce the partial sums
    sum = std::accumulate(partial_sums.begin(), partial_sums.end(), static_cast<real>(0.0));
    sum2 = std::accumulate(partial_sums_squares.begin(), partial_sums_squares.end(), static_cast<real>(0.0));
}

[[maybe_unused]] // not used in the current implementation, bitonic sort is faster
void GPU_data_processing::merge_sort(std::vector<real> &arr, size_t n) {

    // create buffers - copy of the whole array here to make it functional with the current implementation with bitonic sort
    cl::Buffer buffer_arr(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(real) * n, arr.data());
    cl::Buffer buffer_temp(context, CL_MEM_READ_WRITE, sizeof(real) * n);

    // create kernel
    cl::Kernel kernel_merge_sort(program, "merge_sort");

    // execute the kernel
    for (size_t width = 1; width < n; width *= 2) { // for each width
        kernel_merge_sort.setArg(0, buffer_arr);
        kernel_merge_sort.setArg(1, buffer_temp);
        kernel_merge_sort.setArg(2, static_cast<unsigned int>(width));
        kernel_merge_sort.setArg(3, static_cast<unsigned int>(n));

        cl::NDRange global((n+2*width-1) / (width * 2));
        queue.enqueueNDRangeKernel(kernel_merge_sort, cl::NullRange, global);
        queue.finish();
    }

    // read the sorted data back to the host
    queue.enqueueReadBuffer(buffer_arr, CL_TRUE, 0, sizeof(real) * n, arr.data());
}

void GPU_data_processing::bitonic_sort(std::vector<real> &arr, size_t n) {

    // create the kernel
    cl::Kernel bitonic_sort_kernel(program, "bitonic_sort_kernel");
    bitonic_sort_kernel.setArg(0, padded_buffer_arr);

    // calculate the number of stages
    unsigned int num_stages = 0;
    for (size_t i = padded_size; i > 1; i >>= 1)
        ++num_stages;

    size_t local_size = WORK_GROUP_SIZE;
    size_t global_size = ((padded_size >> 1) + local_size - 1) / local_size * local_size;

    // execute the bitonic sort kernel
    for (unsigned int stage = 0; stage < num_stages; ++stage) { // for each stage
        bitonic_sort_kernel.setArg(1, stage);
        for (unsigned int pass_of_stage = 0; pass_of_stage <= stage; ++pass_of_stage) { // for each pass of the stage
            bitonic_sort_kernel.setArg(2, pass_of_stage);
            queue.enqueueNDRangeKernel(bitonic_sort_kernel, cl::NullRange, cl::NDRange(global_size), cl::NDRange(local_size));
            queue.finish();
        }
    }

    // read the sorted data back to the host
    queue.enqueueReadBuffer(padded_buffer_arr, CL_TRUE, 0, sizeof(real) * padded_size, arr.data());

    // remove the padding - padding is at the end of the array
    arr.resize(n);
}

int GPU_data_processing::compute_CV_MAD(std::vector<real> &vec, real &cv, real &mad, bool is_vectorized,
                                        const execution_policy &policy) {
    // to avoid warnings
    (void) is_vectorized;
    (void) policy;

    // initialize the variables
    real sum = 0;
    real sum2 = 0;
    size_t n = vec.size();

    // set the buffer
    set_buffer(vec);

    // compute sums
    sum_vector(sum, sum2, n);

    // sort the data
    auto [sort_time, _] = measure_time([this, &vec, n]() {
        this->bitonic_sort(vec, n);
        return EXIT_SUCCESS; // Ensure the lambda returns a value
    });

    // if sorting was successful calculate the coefficient of variance and median absolute deviation
    if (std::is_sorted(vec.begin(), vec.end())) {
        std::cout << "Sorted in " << sort_time << " seconds" << std::endl;

        mad = (vec[n / 2] + vec[(n - 1) / 2]) / static_cast<real>(2.0);
        abs_diff_calc(vec, mad, n);
        mad = find_median(vec, n);
        cv = CV(sum, sum2, n);

        return EXIT_SUCCESS;

    } else {
        std::cerr << "Failed to sort data" << std::endl;
        return EXIT_FAILURE;
    }

}

