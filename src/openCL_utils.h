#pragma once

#include <CL/cl.hpp>
#include <vector>

#ifdef _MSC_VER
#pragma comment(lib, "opencl.lib")
#endif

/**
 * @brief Setup OpenCL context, command queue and program
 * @param context OpenCL context
 * @param queue OpenCL command queue
 * @param program OpenCL program
 * @param kernelSource OpenCL kernel source code
 * @param kernelName OpenCL kernel name
 */
void setupOpenCL(cl::Context &context, cl::CommandQueue &queue, cl::Program &program, const char* kernelSource, const char* kernelName);

/**
 * @brief Try to select first GPU device
 * @return OpenCL device
 */
cl::Device try_select_first_gpu();

/**
 * @brief Dump build log for OpenCL program
 * @param program OpenCL program
 */
void dump_build_log(cl::Program& program);