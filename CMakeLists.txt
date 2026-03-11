cmake_minimum_required(VERSION 3.17)

project(ParallelCounter)

set(CMAKE_C_STANDATD 11)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)
find_package(OpenMP REQUIRED COMPONENTS C)

add_subdirectory(philosophers)
add_subdirectory(counter)
add_subdirectory(synchronization)
add_subdirectory(openmp)
