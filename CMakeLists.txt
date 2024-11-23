add_definitions(-D__STDC_WANT_SECURE_LIB__=1)

cmake_minimum_required(VERSION 3.21)
project(SP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall /openmp /arch:AVX2")
find_package(OpenCL REQUIRED)

include_directories(src
        src/utils
        src/data_loader
        src/sort/cpu
        src/sort/gpu
        src/statistics/cpu
        src/statistics/gpu
        lib
        lib/drawing)

add_executable(SP
        src/sort/cpu/merge_sort.cpp
        src/sort/cpu/merge_sort.h
        src/main.cpp
        src/data_loader/data_loader.h
        src/data_loader/data_loader.cpp
        src/utils/my_utils.cpp
        src/utils/my_utils.h
        src/statistics/cpu/statistics.cpp
        src/statistics/cpu/statistics.h
        src/openCL.cpp
        src/openCL.h
        src/openCL.h
        src/openCL_utils.cpp
        src/openCL_utils.h
        src/openCL_utils.h
        src/statistics/gpu/GPU_computations.cpp
        src/statistics/gpu/GPU_computations.h
        lib/drawing/Drawing.cpp
        lib/drawing/Drawing.h
        lib/drawing/IRenderer.h
        lib/drawing/SVGRenderer.cpp
        lib/drawing/SVGRenderer.h
        src/svg_ploter/svg_ploter.cpp
        src/svg_ploter/svg_ploter.h
        src/svg_ploter/svg_ploter.h
)

target_link_libraries(SP OpenCL::OpenCL)
