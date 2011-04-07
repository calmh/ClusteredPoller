#ifndef MULTITHREAD_H_
#define MULTITHREAD_H_

#include <pthread.h>

struct mt_context {
        unsigned thread_id;
        pthread_t pthread;
        void *param;
};

struct mt_threads {
        unsigned nthreads;
        struct mt_context *contexts;
};

struct mt_threads *mt_threads_create(unsigned nthreads);
void mt_threads_free(struct mt_threads *threads);
void mt_threads_start(struct mt_threads *threads, void *(*runner) (void *));
void mt_threads_join(struct mt_threads *threads);

#endif                          /* MULTITHREAD_H_ */
