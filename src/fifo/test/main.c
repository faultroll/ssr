
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "cfifo.h"
#include "catomic.h"

static ATOMIC_VAR(int) g_wait = -1;

struct job {
    cfifo_s *results;
    size_t min_results;
};

void *worker(void *arg)
{
    struct job *job = arg;
    int i = pthread_self() % 23, j = -1;
    int tries = 10;
    int total_pop = 0, total_push = 0;

    ATOMIC_VAR_FAA(&g_wait, -1);
    do {} while (g_wait);

    while (tries-- && cfifo_size(job->results) < job->min_results) {
        if (cfifo_push(job->results, (void *)(intptr_t)(++i)) != 0) {
            printf("(%d) push error\n", pthread_self());
            abort();
        } else {
            // printf("(%d) push (%d)\n", pthread_self(), i);
            total_push += i;
        }

        if ((j = (int)(intptr_t)cfifo_pop(job->results)) == (intptr_t)NULL) {
            // printf("(%d) pop error\n", pthread_self());
            continue;
        } else {
            // printf("(%d) pop (%d)\n", pthread_self(), j);
            total_pop += j;
        }
    }

    printf("(%d) total: push (%d), pop (%d)\n", pthread_self(),
           total_push, total_pop);

    return (void *)0;
}

int main()
{
    int nthreads = sysconf(_SC_NPROCESSORS_ONLN); // Linux
    struct job job = {
        .results = NULL,
        .min_results = 4096,
    };
    job.results = cfifo_alloc(job.min_results + nthreads);
    g_wait = nthreads;

    /* Spawn threads. */
    pthread_t threads[nthreads];
    printf("Using %d thread%s.\n", nthreads, nthreads == 1 ? "" : "s");
    for (int i = 0; i < nthreads; i++)
        pthread_create(threads + i, NULL, worker, &job);

    for (int i = 0; i < nthreads; i++)
        pthread_join(threads[i], NULL);

    int remain, total_remain = 0;
    while ((remain = (int)(intptr_t)cfifo_pop(job.results)) != (intptr_t)NULL) {
        total_remain += remain;
    }
    printf("remain total: (%d)\n", total_remain);

    cfifo_free(job.results);

    return 0;
}
