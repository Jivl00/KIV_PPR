cmake_minimum_required(VERSION 3.26)
project(SP)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -fopenmp -Wall")

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
        statistics.h)
