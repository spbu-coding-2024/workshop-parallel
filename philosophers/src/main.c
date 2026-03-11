#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PH_COUNT 5

volatile sig_atomic_t terminated = 0;
void terminate(int signo) { 
	(void)signo;
	terminated = 1; 
}

pthread_mutex_t fork_mtxs[PH_COUNT];

void *philosopher(void *param) {
	int id = *(int *)param;
	int fork1_num = (id - 1) % PH_COUNT;
	int fork2_num = id % PH_COUNT;
	if (fork1_num > fork2_num) {
		int tmp = fork1_num;
		fork1_num = fork2_num;
		fork2_num = tmp;
	}
	while (!terminated) {
		printf("%d is thinking\n", id);
		sleep(2.0 * rand() / RAND_MAX);
		printf("%d is hungry\n", id);
		pthread_mutex_lock(&fork_mtxs[fork1_num]);
		printf("%d has a fork\n", id);
		sleep(2.0 * rand() / RAND_MAX);
		pthread_mutex_lock(&fork_mtxs[fork2_num]);
		printf("%d has 2 forks and is eating\n", id);
		sleep(2.0 * rand() / RAND_MAX);
		pthread_mutex_unlock(fork_mtxs + fork2_num);
		printf("%d has put down a fork\n", id);
		pthread_mutex_unlock(fork_mtxs + fork1_num);
	}
	return NULL;
}

int main() {
	pthread_t tids[PH_COUNT];
	int ids[PH_COUNT];

	signal(SIGINT, terminate);

	int err;
	for (int i = 0; i < PH_COUNT; ++i) {
		ids[i] = i + 1;
		if ((err = pthread_mutex_init(fork_mtxs + i, NULL))) return err;
	}
	for (int i = 0; i < PH_COUNT; ++i) {
		if ((err =
			 pthread_create(tids + i, NULL, philosopher, ids + i)))
			return err;
	}
	for (int i = 0; i < PH_COUNT; ++i) {
		if ((err = pthread_join(tids[i], NULL))) return err;
		printf("%d has finished\n", i);
	}
	for (int i = 0; i < PH_COUNT; ++i) {
		if ((err = pthread_mutex_destroy(fork_mtxs + i))) return err;
	}

	return 0;
}
