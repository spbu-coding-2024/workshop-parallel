add_executable(omp_multiplication "${CMAKE_CURRENT_SOURCE_DIR}/src/multiplication.c")
target_link_libraries(omp_multiplication PRIVATE OpenMP::OpenMP_C)
