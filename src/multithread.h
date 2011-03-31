#ifndef MULTITHREAD_H_
#define MULTITHREAD_H_

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

        typedef struct {
                unsigned thread_id;
                pthread_t pthread;
                void* param;
        } mt_context;

        typedef struct {
                unsigned nthreads;
                mt_context* contexts;
        } mt_threads;

        mt_threads* mt_threads_create(unsigned nthreads);
        void mt_threads_start(mt_threads* threads, void*(*runner)(void*));
        void mt_threads_join(mt_threads* threads);

#ifdef __cplusplus
}
#endif

#endif /* MULTITHREAD_H_ */
