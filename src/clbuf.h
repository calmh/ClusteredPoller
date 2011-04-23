//
//  ClusteredPoller
//
//  Created by Jakob Borg.
//  Copyright 2011 Nym Networks. See LICENSE for terms.
//

#ifndef CLBUF_H
#define CLBUF_H

/// @file clbuf.h
/// A circular buffer, storing pointers (void*) to some opaque object.
/// The actual object at the end of the stored pointer is never touched or freed.

struct clbuf;

#include <pthread.h>

/// Create a new clbuf object.
/// @param size The maximum number of entries the buffer can hold.
struct clbuf *clbuf_create(unsigned size);

/// Free a clbuf object.
/// @param cb The clbuf object.
void clbuf_free(struct clbuf *cb);

/// Stores a pointer into the buffer.
/// @param cb The clbuf object.
/// @param ptr The pointer to store.
/// @return Pointer if successfull, or NULL if there was no space left in the buffer.
void *clbuf_push(struct clbuf *cb, void *ptr);

/// Pops a value from the clbuf.
/// @param cb The clbuf object.
/// @return A value if successfull, or NULL if the buffer was empty.
void *clbuf_pop(struct clbuf *cb);

/// Counts the number of entries currently in use in the clbuf.
/// @param cb The clbuf object.
unsigned clbuf_count_used(struct clbuf *cb);

/// Counts the number of free slots currently available in the clbuf.
/// @param cb The clbuf object.
unsigned clbuf_count_free(struct clbuf *cb);

#endif /* CLBUF_H */
