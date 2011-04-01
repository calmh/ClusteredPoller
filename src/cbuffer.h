//
//  cbuffer.h
//  clpoll
//
//  Created by Jakob Borg on 2011-04-01.
//  Copyright 2011 Nym Networks. All rights reserved.
//

#ifndef CBUFFER_H
#define CBUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

        typedef struct {
                unsigned allocated_size;
                unsigned read_index;
                unsigned write_index;
                void **buffer;
        } cbuffer;

        cbuffer *cbuffer_create(unsigned size);
        void *cbuffer_push(cbuffer *cb, void *ptr);
        void *cbuffer_pop(cbuffer *cb);
        unsigned cbuffer_count(cbuffer *cb);
        unsigned cbuffer_free(cbuffer *cb);

#ifdef __cplusplus
}
#endif

#endif /* CBUFFER_H */