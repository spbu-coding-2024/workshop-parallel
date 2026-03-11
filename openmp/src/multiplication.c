#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

static void parallel_result_calculation(double *restrict matrix,
					double *restrict vector,
					double *restrict result, size_t size);
static void parallel_result_calculation2(double *restrict matrix,
					 double *restrict vector,
					 double *restrict result, size_t size);
static void parallel_result_calculation3(double *restrict matrix,
					 double *restrict vector,
					 double *restrict result, size_t size);
static void parallel_result_calculation4(double *restrict matrix,
					 double *restrict vector,
					 double *restrict result, size_t size);

static double rand_double() {
	const double min = -100.0;
	const double max = 100.0;
	const double div = RAND_MAX / (max - min);
	return min + (rand() / div);
}

static void random_data_initialization(double *restrict matrix,
				       double *restrict vector, size_t size) {
	for (size_t i = 0; i < size; ++i) {
		for (size_t j = 0; j < size; ++j) {
			matrix[i * size + j] = rand_double();
		}
		vector[i] = rand_double();
	}
}

static void reset_result(double *result, size_t size) {
	for (size_t i = 0; i < size; ++i) {
		result[i] = 0.0;
	}
}

static void process_initialization(double **matrix, double **vector,
				   double **result, size_t *size) {
	printf("\nEnter size of the initial objects: ");
	scanf("%zu", size);
	*matrix = malloc((*size) * (*size) * sizeof(**matrix));
	*vector = malloc((*size) * sizeof(**vector));
	*result = calloc(*size, sizeof(**result));
	random_data_initialization(*matrix, *vector, *size);
}

static void serial_result_calculation(double *restrict matrix,
				      double *restrict vector,
				      double *restrict result, size_t size) {
	size_t i, j;  // Loop variables
	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++) {
			result[i] += matrix[i * size + j] * vector[j];
		}
	}
}

static void process_termination(double *restrict matrix,
				double *restrict vector,
				double *restrict result) {
	free(matrix);
	free(vector);
	free(result);
}

static void test_result(double *restrict serial_result, double *restrict result,
			size_t size) {
	double accuracy = 1.0e-6;
	size_t equal = 0;
	// Serial matrix-vector multiplication
	// Testing multiplication results
	for (size_t i = 0; i < size; ++i) {
		if (fabs(result[i] - serial_result[i]) <= accuracy) {
			equal++;
		}
	}
	if (equal < size) {
		printf("The result is NOT correct \n");
	} else {
		printf("The result is correct \n");
	}
}

int main(void) {
	double *matrix;	 // Initial matrix
	double *vector;	 // Initial vector
	double *result;	 // Result vector for matrix-vector multiplication
	size_t size;
	printf("Hello from process with %d threand\n", omp_get_max_threads());

	int thread_id;
#pragma omp parallel private(thread_id)
	{
		thread_id = omp_get_thread_num();

		for (int i = 0; i < omp_get_max_threads(); ++i) {
			if (i == omp_get_thread_num()) {
				printf("Hello from thread: %d\n", thread_id);
			}
#pragma omp barrier
		}
	}
	// Sizes of initial matrix and vector
	// Data initialization
	process_initialization(&matrix, &vector, &result, &size);

	double time_start, time_end;
	// Matrix-vector multiplication
	double *serial_result = calloc(size, sizeof(*serial_result));

	printf("Serial\n");
	time_start = omp_get_wtime();
	serial_result_calculation(matrix, vector, serial_result, size);
	time_end = omp_get_wtime();
	printf("Time elapsed: %f\n", time_end - time_start);

	printf("Parallel 1\n");
	time_start = omp_get_wtime();
	parallel_result_calculation(matrix, vector, result, size);
	time_end = omp_get_wtime();
	printf("Time elapsed: %f\n", time_end - time_start);
	test_result(serial_result, result, size);
	reset_result(result, size);

	/* printf("Parallel 2\n");
	time_start = omp_get_wtime();
	parallel_result_calculation2(matrix, vector, result, size);
	time_end = omp_get_wtime();
	printf("Time elapsed: %f\n", time_end - time_start);
	test_result(serial_result, result, size);
	reset_result(result, size); */

	printf("Parallel 3\n");
	time_start = omp_get_wtime();
	parallel_result_calculation3(matrix, vector, result, size);
	time_end = omp_get_wtime();
	printf("Time elapsed: %f\n", time_end - time_start);
	test_result(serial_result, result, size);
	reset_result(result, size);

	printf("Parallel 4\n");
	time_start = omp_get_wtime();
	parallel_result_calculation4(matrix, vector, result, size);
	time_end = omp_get_wtime();
	printf("Time elapsed: %f\n", time_end - time_start);
	test_result(serial_result, result, size);
	reset_result(result, size);

	// Program termination
	free(serial_result);
	process_termination(matrix, vector, result);
	return 0;
}

static void parallel_result_calculation(double *restrict matrix,
					double *restrict vector,
					double *restrict result, size_t size) {
	size_t i, j;  // Loop variables
	for (i = 0; i < size; i++) {
#pragma omp parallel for
		for (j = 0; j < size; j++) {
			result[i] += matrix[i * size + j] * vector[j];
		}
	}
}

void parallel_result_calculation2(double *restrict matrix,
				  double *restrict vector,
				  double *restrict result, size_t size) {
	size_t i, j;  // Loop variables
	omp_lock_t my_lock;
	omp_init_lock(&my_lock);
	for (i = 0; i < size; i++) {
#pragma omp parallel for
		for (j = 0; j < size; j++) {
			omp_set_lock(&my_lock);
			result[i] += matrix[i * size + j] * vector[j];
			omp_unset_lock(&my_lock);
		}
	}
	omp_destroy_lock(&my_lock);
}

void parallel_result_calculation3(double *restrict matrix,
				  double *restrict vector,
				  double *restrict result, size_t size) {
	size_t i, j;  // Loop variables
	double iter_global_sum = 0;
	for (i = 0; i < size; i++) {
		iter_global_sum = 0;
#pragma omp parallel for reduction(+ : iter_global_sum)
		for (j = 0; j < size; j++) {
			iter_global_sum += matrix[i * size + j] * vector[j];
		}
		result[i] = iter_global_sum;
	}
}

static void parallel_result_calculation4(double *restrict matrix,
					 double *restrict vector,
					 double *restrict result, size_t size) {
	double *all_results =
	    calloc(size * omp_get_max_threads(), sizeof(*all_results));
	size_t thread_num;
#pragma omp parallel
	{
		thread_num = (size_t)omp_get_num_threads();
		int thread_id = omp_get_thread_num();
		size_t block_size = size / thread_num;
		double iter_result;
		for (size_t i = 0; i < size; ++i) {
			iter_result = 0;
			for (size_t j = 0; j < block_size; ++j)
				iter_result +=
				    matrix[i * size + j +
					   thread_id * block_size] *
				    vector[j + thread_id * block_size];
			all_results[size * thread_id + i] = iter_result;
		}
	}
	for (size_t i = 0; i < size; i++)
		for (size_t j = 0; j < thread_num; j++)
			result[i] += all_results[j * size + i];
	free(all_results);
}
