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

struct clbuf {
        pthread_mutex_t lock;
        unsigned allocated_size;
        unsigned read_index;
        unsigned write_index;
        void **buffer;
};

struct clbuf *clbuf_create(unsigned size);
void *clbuf_push(struct clbuf *cb, void *ptr);
void *clbuf_pop(struct clbuf *cb);
unsigned clbuf_count(struct clbuf *cb);
unsigned clbuf_free(struct clbuf *cb);

#endif /* CBUFFER_H */
