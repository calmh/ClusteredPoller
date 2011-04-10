//
//  clbuf.h
//  clpoll
//
//  Created by Jakob Borg on 2011-04-01.
//  Copyright 2011 Nym Networks. All rights reserved.
//

#ifndef CBUFFER_H
#define CBUFFER_H

#include <pthread.h>

// A circular buffer, storing pointers to some opaque object.
struct clbuf {
        pthread_mutex_t lock;
        unsigned allocated_size;
        unsigned read_index;
        unsigned write_index;
        void **buffer;
};

// Create a new clbuf object.
// size -- the maximum number of entries the buffer can hold.
struct clbuf *clbuf_create(unsigned size);

// Free a clbuf object.
void clbuf_free(struct clbuf *cb);

// Stores a pointer into the buffer.
// ptr -- the pointer to store.
// Returns ptr if successfull, or NULL if there was no space.
void *clbuf_push(struct clbuf *cb, void *ptr);

// Pops a value from the clbuf.
// Returns a value if successfull, or NULL if the buffer was empty.
void *clbuf_pop(struct clbuf *cb);

// Counts the number of entries currently in use in the clbuf.
unsigned clbuf_count_used(struct clbuf *cb);

// Counts the number of free slots currently available in the clbuf.
unsigned clbuf_count_free(struct clbuf *cb);

#endif                          /* CBUFFER_H */
