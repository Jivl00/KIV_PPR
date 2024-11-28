add_definitions(-D__STDC_WANT_SECURE_LIB__=1)

cmake_minimum_required(VERSION 3.21)
project(SP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /openmp /arch:AVX2")
find_package(OpenCL REQUIRED)

include_directories(src
        src/utils
        src/data_loader
        src/data_processing/CPU/
        src/data_processing/GPU/
        src/statistics/cpu
        src/statistics/gpu
        lib
        lib/drawing)

add_executable(SP
        src/data_processing/CPU/merge_sort.cpp
        src/data_processing/CPU/merge_sort.h
        src/main.cpp
        src/data_loader/data_loader.h
        src/data_loader/data_loader.cpp
        src/utils/my_utils.h
        src/data_processing/CPU/statistics.cpp
        src/data_processing/CPU/statistics.h
        src/data_processing/GPU/openCL.cpp
        src/data_processing/GPU/openCL.h
        src/data_processing/GPU/openCL.h
        src/data_processing/GPU/GPU_calc.cpp
        src/data_processing/GPU/GPU_calc.h
        lib/drawing/Drawing.cpp
        lib/drawing/Drawing.h
        lib/drawing/IRenderer.h
        lib/drawing/SVGRenderer.cpp
        lib/drawing/SVGRenderer.h
        src/svg_ploter/svg_ploter.cpp
        src/svg_ploter/svg_ploter.h
        src/svg_ploter/svg_ploter.h
        src/utils/device_type.h
        src/utils/execution_policy.h
)

target_link_libraries(SP OpenCL::OpenCL)
