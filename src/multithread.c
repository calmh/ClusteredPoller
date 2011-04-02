#include <stdlib.h>
#include <pthread.h>

#include "multithread.h"

#define THREAD_STACK_SIZE (256*1024)

mt_threads *mt_threads_create(unsigned nthreads)
{
        mt_threads *threads = (mt_threads *) malloc(sizeof(mt_threads));
        if (!threads)
                return NULL;
        threads->contexts = (mt_context *) malloc(sizeof(mt_context) * nthreads);
        threads->nthreads = nthreads;
        unsigned i;
        for (i = 0; i < nthreads; i++)
                threads->contexts[i].thread_id = i;
        return threads;
}

void mt_threads_free(mt_threads *threads)
{
        free(threads->contexts);
        free(threads);
}

void mt_threads_start(mt_threads *threads, void*(*runner)(void *))
{
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, THREAD_STACK_SIZE);

        unsigned i;
        for (i = 0; i < threads->nthreads; i++) {
                pthread_create(&threads->contexts[i].pthread, &attr, runner, &threads->contexts[i]);
        }
}

void mt_threads_join(mt_threads *threads)
{
        unsigned i;
        for (i = 0; i < threads->nthreads; i++)
                pthread_join(threads->contexts[i].pthread, NULL);
}

