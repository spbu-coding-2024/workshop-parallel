#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>

#define WORKERS_COUNT 4

pthread_t workers[WORKERS_COUNT] = {0};

size_t counter = 0;
size_t iteration = 0;

void *work(void *param) {
	for (size_t i = 0; i < 10000000; ++i) {
		counter += i;
	}
	return NULL;
}

void example() {
	counter = 0;
	for (size_t i = 0; i < WORKERS_COUNT; ++i) {
		pthread_create(workers + i, NULL, work, NULL);
	}
	for (size_t i = 0; i < WORKERS_COUNT; ++i) {
		pthread_join(workers[i], NULL);
	}
	if (counter != 199999980000000) {
		printf("counter = %lu\niteration = %lu\n", counter, iteration);
	}
	iteration++;
	return;
}

int main() {
	const size_t iterations = 1000000;
	for (size_t i = 0; i < iterations; ++i) {
		example();
	}
	return 0;
}
