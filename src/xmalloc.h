/*
 *  ClusteredPoller
 *
 *  Created by Jakob Borg.
 *  Copyright 2011 Nym Networks. See LICENSE for terms.
 */

#ifndef XMALLOC_H
#define XMALLOC_H

/** @file xmalloc.h Checked malloc and xrealloc */

/**
 * Malloc that will (attempt to) log and abort in case of failure.
 * @param size Number of bytes to allocate.
 * @return Valid allocation.
 */
void *xmalloc(size_t size);

/**
 * xrealloc that will (attempt to) log and abort in case of failure.
 * @param ptr Pointer to reallocate.
 * @param size Number of bytes to allocate.
 * @return Valid allocation.
 */
void *xrealloc(void *ptr, size_t size);

#endif /* XALLOC_H */
