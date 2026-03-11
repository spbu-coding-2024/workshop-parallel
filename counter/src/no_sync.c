#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>

#define WORKERS_COUNT 4

pthread_t workers[WORKERS_COUNT] = {0};

size_t counter = 0;

void *work(void *param) {
	for (size_t i = 0; i < 10000000; ++i) {
		counter += i;
	}
	return NULL;
}

double example() {
	struct timespec t_start, t_end;
	counter = 0;
	clock_gettime(CLOCK_MONOTONIC, &t_start);
	for (size_t i = 0; i < WORKERS_COUNT; ++i) {
		pthread_create(workers + i, NULL, work, NULL);
	}
	for (size_t i = 0; i < WORKERS_COUNT; ++i) {
		pthread_join(workers[i], NULL);
	}
	clock_gettime(CLOCK_MONOTONIC, &t_end);
	printf("counter = %lu\n", counter);
	return (t_end.tv_sec - t_start.tv_sec) +
	       (t_end.tv_nsec - t_start.tv_nsec) / 1000000000.0;
}

int main() {
	const size_t iterations = 10;
	double sum_time = 0.0;
	for (size_t i = 0; i < iterations; ++i) {
		sum_time += example();
	}
	printf("mean time = %f\n", sum_time / iterations);
	return 0;
}
