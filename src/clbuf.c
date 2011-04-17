//
//  ClusteredPoller
//
//  Created by Jakob Borg.
//  Copyright 2011 Nym Networks. See LICENSE for terms.
//

#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "clbuf.h"
#include "xmalloc.h"

struct clbuf *clbuf_create(unsigned size)
{
        struct clbuf *cb = (struct clbuf *) xmalloc(sizeof(struct clbuf));

        // Initialize the lock as a recursive lock.
        pthread_mutexattr_t mta;
        pthread_mutexattr_init(&mta);
        pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&cb->lock, &mta);

        // Initialize indexes and allocate a buffer with the specified size.
        cb->allocated_size = size;
        cb->read_index = 0;
        cb->write_index = 0;
        cb->buffer = (void **) xmalloc(sizeof(void *) * size);
        memset(cb->buffer, 0, sizeof(void *) * size);
        return cb;
}

void clbuf_free(struct clbuf *cb)
{
        free(cb->buffer);
        free(cb);
}

void *clbuf_push(struct clbuf *cb, void *ptr)
{
        // Will not push a NULL pointer.
        if (!ptr)
                return NULL;

        // Lock the buffer for modification.
        pthread_mutex_lock(&cb->lock);

        // Check if there is space to push another item into the buffer.
        // If not, unlock and return NULL.
        if (cb->write_index == cb->read_index && cb->buffer[cb->write_index] != 0) {
                // The buffer is full.
                pthread_mutex_unlock(&cb->lock);
                return NULL;
        }
        // Push the item into the buffer and increment the write index pointer.
        cb->buffer[cb->write_index] = ptr;
        cb->write_index++;
        cb->write_index %= cb->allocated_size;

        // Unlock the buffer and return success.
        pthread_mutex_unlock(&cb->lock);
        return ptr;
}

void *clbuf_pop(struct clbuf *cb)
{
        // Lock the buffer for modification.
        pthread_mutex_lock(&cb->lock);

        // Check if there is anything to pop.
        // If not, unlock and return NULL.
        if (cb->write_index == cb->read_index && cb->buffer[cb->read_index] == 0) {
                // The buffer is empty.
                pthread_mutex_unlock(&cb->lock);
                return NULL;
        }
        // Get the pointer to pop, and zeroize that position in the buffer.
        void *ptr = cb->buffer[cb->read_index];
        cb->buffer[cb->read_index] = NULL;

        // Increment the read index pointer.
        cb->read_index++;
        cb->read_index %= cb->allocated_size;

        // Unlock and return the popped item.
        pthread_mutex_unlock(&cb->lock);
        return ptr;
}

unsigned clbuf_count_used(struct clbuf *cb)
{
        unsigned count;

        // Lock the buffer so it doesn't change under our feet.
        pthread_mutex_lock(&cb->lock);

        // If the read and write indexes are the same, and that item is 0, the buffer is empty.
        if (cb->write_index == cb->read_index && cb->buffer[cb->read_index] == 0)
                count = 0;
        // If the read and write indexes are the same, and that item is not 0, the buffer is full.
        else if (cb->write_index == cb->read_index && cb->buffer[cb->read_index] != 0)
                count = cb->allocated_size;
        // If the write index is ahead of the read index, the difference is the number of items in the buffer.
        else if (cb->write_index >= cb->read_index)
                count = cb->write_index - cb->read_index;
        // Otherwise, we wrapped and need to add the buffer size to the difference.
        else
                count = cb->write_index + cb->allocated_size - cb->read_index;

        // Unlock and return the result.
        pthread_mutex_unlock(&cb->lock);
        return count;
}

unsigned clbuf_count_free(struct clbuf *cb)
{
        pthread_mutex_lock(&cb->lock);
        // The number of free items is the buffer size minus the number of items stored.
        unsigned count_free = cb->allocated_size - clbuf_count_used(cb);
        pthread_mutex_unlock(&cb->lock);
        return count_free;
}
