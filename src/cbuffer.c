//
//  cbuffer.c
//  clpoll
//
//  Created by Jakob Borg on 2011-04-01.
//  Copyright 2011 Nym Networks. All rights reserved.
//

#include <stdlib.h>
#include <string.h>
#include "cbuffer.h"

cbuffer *cbuffer_create(unsigned size)
{
        cbuffer *cb = (cbuffer *) malloc(sizeof(cbuffer));
        cb->allocated_size = size;
        cb->read_index = 0;
        cb->write_index = 0;
        cb->buffer = (void **) malloc(sizeof(void *) * size);
        memset(cb->buffer, 0, sizeof(void *) * size);
        return cb;
}

void *cbuffer_push(cbuffer *cb, void *ptr)
{
        if (!ptr)
                return NULL;
        if (cb->write_index == cb->read_index && cb->buffer[cb->write_index] != 0)
                return NULL; /* Buffer is full */
        cb->buffer[cb->write_index] = ptr;
        cb->write_index++;
        cb->write_index %= cb->allocated_size;
        return ptr;
}

void *cbuffer_pop(cbuffer *cb)
{
        if (cb->write_index == cb->read_index && cb->buffer[cb->read_index] == 0)
                return NULL; /* Buffer is empty */
        void *ptr = cb->buffer[cb->read_index];
        cb->buffer[cb->read_index] = NULL;
        cb->read_index++;
        cb->read_index %= cb->allocated_size;
        return ptr;
}

unsigned cbuffer_count(cbuffer *cb)
{
        if (cb->write_index == cb->read_index && cb->buffer[cb->read_index] == 0)
                return 0;
        if (cb->write_index == cb->read_index && cb->buffer[cb->read_index] != 0)
                return cb->allocated_size;
        if (cb->write_index >= cb->read_index)
                return cb->write_index - cb->read_index;
        return (cb->write_index + cb->allocated_size - cb->read_index);
}

unsigned cbuffer_free(cbuffer *cb)
{
        return cb->allocated_size - cbuffer_count(cb);
}
