add_executable(pthread_philosophers "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c")
target_link_libraries(pthread_philosophers PRIVATE Threads::Threads)
