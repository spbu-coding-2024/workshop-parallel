add_executable(pthread_synchronization "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c")
target_link_libraries(pthread_synchronization PRIVATE Threads::Threads)
