cmake_minimum_required(VERSION 3.26)
project(SP)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -fopenmp -Wall")

include_directories(.)

add_executable(SP
        mergeSort.cpp
        mergeSort.h
        main.cpp
        dataLoader.h
        dataLoader.cpp
        my_utils.cpp
        my_utils.h
        statistics.cpp
        statistics.h)
