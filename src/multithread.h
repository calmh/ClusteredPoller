/*
 *  ClusteredPoller
 *
 *  Created by Jakob Borg.
 *  Copyright 2011 Nym Networks. See LICENSE for terms.
 */

#ifndef MULTITHREAD_H
#define MULTITHREAD_H

/** @file multithread.h Clpoll multithreading implementation. */

#include <pthread.h>

/** A thread context object. Each thread gets one. */
struct mt_context {
        unsigned thread_id;     /**< The thread ID, incremented by one for each new thread. */
        pthread_t pthread;      /**< The pthread_t object for this thread. */
        void *param;            /**< Optional parameter pointer, for use by the thread. */
};

/**
 * A thread collection object.
 * @see mt_threads_create
 * @see mt_threads_free
 * @see mt_threads_start
 * @see mt_threads_join
 */
struct mt_threads {
        unsigned nthreads;      /**< The number of threads. */
        struct mt_context *contexts;    /**< A thread context per thread. */
};

/**
 * Create a new mt_threads object.
 * @param nthreads The number of threads to create.
 * @return A new mt_threads object.
 */
struct mt_threads *mt_threads_create(unsigned nthreads);

/**
 * Free an mt_threads object.
 * @param threads The object to free.
 */
void mt_threads_free(struct mt_threads *threads);

/**
 * Start all threads in the mt_threads object.
 * @param threads The mt_threads object to run.
 * @param runner The mainloop to call for each thread.
 */
void mt_threads_start(struct mt_threads *threads, void *(*runner) (void *));

/**
 * Wait for all threads in the mt_threads object to finish.
 * @param threads The threads to wait for.
 */
void mt_threads_join(struct mt_threads *threads);

#endif /* MULTITHREAD_H */
