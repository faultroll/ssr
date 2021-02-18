
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "clifo.h"
#include "catomic.h"

#define MAX_TRIES (512)
static ATOMIC_VAR(int) g_toal_push = ATOMIC_VAR_INIT(0),
                       g_toal_pop = ATOMIC_VAR_INIT(0);
static ATOMIC_VAR(int) g_wait = ATOMIC_VAR_INIT(-1);
static ATOMIC_FLAG g_spin = ATOMIC_FLAG_INIT;
#define SPIN_LOCK(_lock) do { while (!ATOMIC_FLAG_TAS(&(_lock))) {} } while (0)
#define SPIN_UNLOCK(_lock) do { ATOMIC_FLAG_CLR(&(_lock)); } while (0)

struct job {
    clifo_s *results;
    size_t min_results;
};

void *worker(void *arg)
{
    struct job *job = arg;
    int i = pthread_self() % 23, j = -1;
    int tries = MAX_TRIES;
    int total_pop = 0, total_push = 0;
    // int in[MAX_TRIES] = {0}, out[MAX_TRIES] = {0};

    ATOMIC_VAR_FAA(&g_wait, -1);
    do {} while (ATOMIC_VAR_LOAD(&g_wait));

    while (tries-- && clifo_size(job->results) < job->min_results) {
#if 0 /* push first */
        if (clifo_push(job->results, (void *)(intptr_t)(++i)) != 0) {
            printf("(%d) push error\n", pthread_self());
            abort();
        } else {
            // printf("(%d) push (%d)\n", pthread_self(), i);
            // in[tries] = i;
            total_push += i;
        }

        if ((j = (int)(intptr_t)clifo_pop(job->results)) == (intptr_t)NULL) {
            printf("(%d) pop error\n", pthread_self());
            // continue;
        } else {
            // printf("(%d) pop (%d)\n", pthread_self(), j);
            // out[tries] = j;
            total_pop += j;
        }
#else /* pop first */
        if ((j = (int)(intptr_t)clifo_pop(job->results)) == (intptr_t)NULL) {
            printf("(%d) pop error\n", pthread_self());
            // continue;
        } else {
            // printf("(%d) pop (%d)\n", pthread_self(), j);
            // out[tries] = j;
            total_pop += j;
        }

        if (clifo_push(job->results, (void *)(intptr_t)(++i)) != 0) {
            printf("(%d) push error\n", pthread_self());
            abort();
        } else {
            // printf("(%d) push (%d)\n", pthread_self(), i);
            // in[tries] = i;
            total_push += i;
        }
#endif
    }

    SPIN_LOCK(g_spin);
    printf("(%d) total: push (%d), pop (%d)\n", pthread_self(),
           total_push, total_pop);
    // printf("(%d) push: (", pthread_self());
    // for (i = 0; i < MAX_TRIES; ++i)
    //     printf("%d, ", in[i]);
    // printf(")\n");
    // printf("(%d) pop: (", pthread_self());
    // for (i = 0; i < MAX_TRIES; ++i)
    //     printf("%d, ", out[i]);
    // printf(")\n");
    SPIN_UNLOCK(g_spin);

    ATOMIC_VAR_FAA(&g_toal_push, total_push);
    ATOMIC_VAR_FAA(&g_toal_pop, total_pop);

    return (void *)0;
}

int main()
{
    int nthreads = sysconf(_SC_NPROCESSORS_ONLN); // Linux
    struct job job = {
        .results = NULL,
        .min_results = 4096,
    };
    job.results = clifo_alloc(job.min_results + nthreads);
    ATOMIC_VAR_STOR(&g_wait, nthreads);

    /* Spawn threads. */
    pthread_t threads[nthreads];
    printf("Using %d thread%s.\n", nthreads, nthreads == 1 ? "" : "s");
    for (int i = 0; i < nthreads; i++)
        pthread_create(threads + i, NULL, worker, &job);

    for (int i = 0; i < nthreads; i++)
        pthread_join(threads[i], NULL);

    int remain, total_remain = 0;
    while ((remain = (int)(intptr_t)clifo_pop(job.results)) != (intptr_t)NULL) {
        total_remain += remain;
    }
    printf("remain total: (%d)\n", total_remain);
    ATOMIC_VAR_FAA(&g_toal_pop, total_remain);
    printf("g_toal_push (%d), g_toal_pop (%d)\n", g_toal_push, g_toal_pop);

    clifo_free(job.results);

    return 0;
}
