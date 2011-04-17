//
//  ClusteredPoller
//
//  Created by Jakob Borg.
//  Copyright 2011 Nym Networks. See LICENSE for terms.
//

#include <stdlib.h>
#include <syslog.h>
#include <stdio.h>

void *xmalloc(size_t size)
{
        void *result = malloc(size);
        if (!result) {
                fprintf(stderr, "Failed to malloc %lu bytes of memory. Aborting.\n", (unsigned long) size);
                syslog(LOG_CRIT, "Failed to malloc %lu bytes of memory. Aborting.\n", (unsigned long) size);
                abort();
        }
        return result;
}

void *xrealloc(void *ptr, size_t size)
{
        void *result = realloc(ptr, size);
        if (!result) {
                fprintf(stderr, "Failed to xrealloc %lu bytes of memory. Aborting.\n", (unsigned long) size);
                syslog(LOG_CRIT, "Failed to xrealloc %lu bytes of memory. Aborting.\n", (unsigned long) size);
                abort();
        }
        return result;
}
