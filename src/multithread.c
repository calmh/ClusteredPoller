#include <stdlib.h>
#include <pthread.h>

#include "multithread.h"

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

void mt_threads_start(mt_threads *threads, void*(*runner)(void *))
{
	unsigned i;
        for (i = 0; i < threads->nthreads; i++)
                pthread_create(&threads->contexts[i].pthread, NULL, runner, &threads->contexts[i]);
}

void mt_threads_join(mt_threads *threads)
{
	unsigned i;
        for (i = 0; i < threads->nthreads; i++)
                pthread_join(threads->contexts[i].pthread, NULL);
}

