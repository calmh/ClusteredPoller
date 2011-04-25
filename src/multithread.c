/*
 *  ClusteredPoller
 *
 *  Created by Jakob Borg.
 *  Copyright 2011 Nym Networks. See LICENSE for terms.
 */

#include <stdlib.h>
#include <pthread.h>

#include "multithread.h"
#include "xmalloc.h"

#define THREAD_STACK_SIZE (256*1024)

struct mt_threads *mt_threads_create(unsigned nthreads)
{
        struct mt_threads *threads = (struct mt_threads *) xmalloc(sizeof(struct mt_threads));
        threads->contexts = (struct mt_context *) xmalloc(sizeof(struct mt_context) * nthreads);
        threads->nthreads = nthreads;
        unsigned i;
        for (i = 0; i < nthreads; i++)
                threads->contexts[i].thread_id = i;
        return threads;
}

void mt_threads_free(struct mt_threads *threads)
{
        free(threads->contexts);
        free(threads);
}

void mt_threads_start(struct mt_threads *threads, void *(*runner) (void *))
{
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, THREAD_STACK_SIZE);

        unsigned i;
        for (i = 0; i < threads->nthreads; i++) {
                pthread_create(&threads->contexts[i].pthread, &attr, runner, &threads->contexts[i]);
        }
}

void mt_threads_join(struct mt_threads *threads)
{
        unsigned i;
        for (i = 0; i < threads->nthreads; i++)
                pthread_join(threads->contexts[i].pthread, NULL);
}
