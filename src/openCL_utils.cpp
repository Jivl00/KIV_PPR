#include "openCL_utils.h"
#include <iostream>

void setupOpenCL(cl::Context &context, cl::CommandQueue &queue, cl::Program &program, const char *kernelSource,
                 const char *kernelName) {
    cl::Device device = try_select_first_gpu();

    std::vector<std::pair<const char *, size_t>> source_codes{{kernelSource, strlen(kernelSource)}};
    const cl::Program::Sources &sources(source_codes);
    context = cl::Context{device};
    program = cl::Program{context, sources};

    program.build({device}, "-cl-std=CL2.0 -cl-denorms-are-zero");
    queue = cl::CommandQueue{context, device};
}

cl::Device try_select_first_gpu() {
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

void dump_build_log(cl::Program &program) {
    cl_int buildErr = CL_SUCCESS;
    auto devices = program.getInfo<CL_PROGRAM_DEVICES>();
    for (auto &device: devices) {
        auto buildInfo = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device, &buildErr);
        if (!buildInfo.empty()) {
            std::wcout << L"Errors and warnings during compilation:" << std::endl;
            std::cout << buildInfo << std::endl << std::endl;
        }
    }
}