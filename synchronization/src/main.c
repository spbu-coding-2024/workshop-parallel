#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PRODUCERS_GROUP_COUNT 2
#define PRODUCERS_IN_GROUP 3
#define CONSUMERS_COUNT 5

int finished = 0;
pthread_rwlock_t finished_rw_lock;

int all_tasks[PRODUCERS_GROUP_COUNT * PRODUCERS_IN_GROUP] = {0};
int group_leaders[PRODUCERS_GROUP_COUNT] = {0};
int group_finished[PRODUCERS_GROUP_COUNT] = {0};
pthread_barrier_t group_barriers[PRODUCERS_GROUP_COUNT];
pthread_spinlock_t leaders_spin[PRODUCERS_GROUP_COUNT];

typedef struct result_record {
	int group_number;
	int value;
} result_t;

result_t result = {.group_number = -1, .value = 0};
int result_ready = 0;
pthread_cond_t result_cond;
pthread_mutex_t result_mutex;

typedef struct producer_parameters {
	int number_in_group;
	int group_number;
	int *tasks;
} pr_params_t;

typedef struct consumer_parameters {
	int id;

} cm_params_t;

void *produce(void *param) {
	pr_params_t *pr_param = (pr_params_t *)param;
	int id = pr_param->number_in_group;
	int *tasks = pr_param->tasks;
	int group = pr_param->group_number;
	int *leader = group_leaders + group;
	pthread_barrier_t *my_barrier = group_barriers + group;
	for (;;) {
		pthread_barrier_wait(my_barrier);
		if (group_finished[group]) break;
		// Do some work
		sleep(2.0 * rand() / RAND_MAX);
		tasks[id] = rand() % 100;
		pthread_barrier_wait(my_barrier);
		// Collect results
		pthread_spin_lock(leaders_spin + group);
		if (*leader == -1) *leader = id;
		pthread_spin_unlock(leaders_spin + group);
		if (*leader != id) continue;
		int group_result = tasks[0] + tasks[1] + tasks[2];
		// Send result
		pthread_mutex_lock(&result_mutex);
		result.group_number = group;
		result.value = group_result;
		result_ready = 1;
		pthread_cond_signal(&result_cond);
		pthread_mutex_unlock(&result_mutex);
		pthread_rwlock_rdlock(&finished_rw_lock);
		if (finished) group_finished[group] = 1;
		pthread_rwlock_unlock(&finished_rw_lock);
		*leader = -1;
	}
	return NULL;
}

void *consume(void *param) {
	cm_params_t *cm_param = (cm_params_t *)param;
	result_t processing_result;
	for (;;) {
		// Get result
		pthread_rwlock_rdlock(&finished_rw_lock);
		if (finished) break;
		pthread_rwlock_unlock(&finished_rw_lock);
		pthread_mutex_lock(&result_mutex);
		if (result_ready) {
			processing_result = result;
			result_ready = 0;
		} else {
			struct timespec stime;
			int now = time(NULL);
			stime.tv_sec = now + 7;
			if (pthread_cond_timedwait(&result_cond, &result_mutex,
						   &stime) == ETIMEDOUT ||
			    !result_ready) {
				pthread_mutex_unlock(&result_mutex);
				continue;
			}
			processing_result = result;
			result_ready = 0;
		}
		pthread_mutex_unlock(&result_mutex);
		// Processing
		sleep(2.0 * rand() / RAND_MAX);
		printf("Consumer %d processed result %d produced by group %d\n",
		       cm_param->id, processing_result.value,
		       processing_result.group_number);
	}
	return NULL;
}

int main() {
	pthread_t producers[PRODUCERS_GROUP_COUNT * PRODUCERS_IN_GROUP];
	pr_params_t pr_params[PRODUCERS_GROUP_COUNT * PRODUCERS_IN_GROUP];

	pthread_t consumers[CONSUMERS_COUNT];
	cm_params_t cm_params[CONSUMERS_COUNT];

	int err;
	// Initialize finished_rw_lock, write_mutex, result_mutex, result_cond
	if ((err = pthread_rwlock_init(&finished_rw_lock, NULL))) return err;
	if ((err = pthread_mutex_init(&result_mutex, NULL))) return err;
	if ((err = pthread_cond_init(&result_cond, NULL))) return err;
	// Initialize barriers and leaders
	for (int i = 0; i < PRODUCERS_GROUP_COUNT; ++i) {
		if ((err = pthread_barrier_init(group_barriers + i, NULL,
						PRODUCERS_IN_GROUP)))
			return err;
		if ((err = pthread_spin_init(leaders_spin + i, 0))) return err;
		group_leaders[i] = -1;
	}
	// Initialize groups
	for (int i = 0; i < PRODUCERS_GROUP_COUNT * PRODUCERS_IN_GROUP; ++i) {
		pr_params[i].number_in_group = i % PRODUCERS_IN_GROUP;
		pr_params[i].group_number = i / PRODUCERS_IN_GROUP;
		pr_params[i].tasks =
		    all_tasks + (i / PRODUCERS_IN_GROUP) * PRODUCERS_IN_GROUP;
	}
	// Initialize producers
	for (int i = 0; i < PRODUCERS_GROUP_COUNT * PRODUCERS_IN_GROUP; ++i) {
		if ((err = pthread_create(producers + i, NULL, produce,
					  pr_params + i)))
			return err;
	}
	// Initialize consumers
	for (int i = 0; i < CONSUMERS_COUNT; ++i) {
		cm_params[i].id = i;
		if ((err = pthread_create(consumers + i, NULL, consume,
					  cm_params + i)))
			return err;
	}
	printf("Press Ctrl+d to terminate\n");
	getchar();
	pthread_rwlock_wrlock(&finished_rw_lock);
	finished = 1;
	pthread_rwlock_unlock(&finished_rw_lock);
	// Join producers
	for (int i = 0; i < PRODUCERS_GROUP_COUNT * PRODUCERS_IN_GROUP; ++i) {
		if ((err = pthread_join(producers[i], NULL))) return err;
	}
	// Join consumers
	for (int i = 0; i < CONSUMERS_COUNT; ++i) {
		if ((err = pthread_join(consumers[i], NULL))) return err;
	}
	// Destroy finished_rw_lock, write_mutex, result_mutex, result_cond
	if ((err = pthread_rwlock_destroy(&finished_rw_lock))) return err;
	if ((err = pthread_mutex_destroy(&result_mutex))) return err;
	if ((err = pthread_cond_destroy(&result_cond))) return err;
	// Destroy barriers and leaders
	for (int i = 0; i < PRODUCERS_GROUP_COUNT; ++i) {
		if ((err = pthread_barrier_destroy(group_barriers + i)))
			return err;
		if ((err = pthread_spin_destroy(leaders_spin + i))) return err;
	}

	return 0;
}
