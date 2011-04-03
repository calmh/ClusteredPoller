//
//  clbuf.c
//  clpoll
//
//  Created by Jakob Borg on 2011-04-01.
//  Copyright 2011 Nym Networks. All rights reserved.
//

#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "clbuf.h"

struct clbuf *clbuf_create(unsigned size) {
        struct clbuf *cb = (struct clbuf *) malloc(sizeof(struct clbuf));

        pthread_mutexattr_t mta;
        pthread_mutexattr_init(&mta);
        pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&cb->lock, &mta);

        cb->allocated_size = size;
        cb->read_index = 0;
        cb->write_index = 0;
        cb->buffer = (void **) malloc(sizeof(void *) * size);
        memset(cb->buffer, 0, sizeof(void *) * size);
        return cb;
}

void *clbuf_push(struct clbuf *cb, void *ptr)
{
        if (!ptr)
                return NULL;
        pthread_mutex_lock(&cb->lock);
        if (cb->write_index == cb->read_index && cb->buffer[cb->write_index] != 0) {
                pthread_mutex_unlock(&cb->lock);
                return NULL; /* Buffer is full */
        }
        cb->buffer[cb->write_index] = ptr;
        cb->write_index++;
        cb->write_index %= cb->allocated_size;
        pthread_mutex_unlock(&cb->lock);
        return ptr;
}

void *clbuf_pop(struct clbuf *cb)
{
        pthread_mutex_lock(&cb->lock);
        if (cb->write_index == cb->read_index && cb->buffer[cb->read_index] == 0) {
                pthread_mutex_unlock(&cb->lock);
                return NULL; /* Buffer is empty */
        }
        void *ptr = cb->buffer[cb->read_index];
        cb->buffer[cb->read_index] = NULL;
        cb->read_index++;
        cb->read_index %= cb->allocated_size;
        pthread_mutex_unlock(&cb->lock);
        return ptr;
}

unsigned clbuf_count(struct clbuf *cb)
{
        unsigned count;
        pthread_mutex_lock(&cb->lock);
        if (cb->write_index == cb->read_index && cb->buffer[cb->read_index] == 0)
                count = 0;
        else if (cb->write_index == cb->read_index && cb->buffer[cb->read_index] != 0)
                count = cb->allocated_size;
        else if (cb->write_index >= cb->read_index)
                count = cb->write_index - cb->read_index;
        else
                count = cb->write_index + cb->allocated_size - cb->read_index;
        pthread_mutex_unlock(&cb->lock);
        return count;
}

unsigned clbuf_free(struct clbuf *cb)
{
        pthread_mutex_lock(&cb->lock);
        unsigned free = cb->allocated_size - clbuf_count(cb);
        pthread_mutex_unlock(&cb->lock);
        return free;
}
