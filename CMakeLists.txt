add_definitions(-D__STDC_WANT_SECURE_LIB__=1)

cmake_minimum_required(VERSION 3.21)
project(SP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall /openmp /arch:AVX2")
find_package(OpenCL REQUIRED)

include_directories(.)

add_executable(SP
        merge_sort.cpp
        merge_sort.h
        main.cpp
        data_loader.h
        data_loader.cpp
        my_utils.cpp
        my_utils.h
        statistics.cpp
        statistics.h
        openCL.cpp
        openCL.h
        openCL.h
        openCL_utils.cpp
        openCL_utils.h
        openCL_utils.h
)

target_link_libraries(SP OpenCL::OpenCL)
