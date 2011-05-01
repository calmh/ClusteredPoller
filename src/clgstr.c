/*
 *  ClusteredPoller
 *
 *  Created by Jakob Borg.
 *  Copyright 2011 Nym Networks. See LICENSE for terms.
 */

#include "clgstr.h"

#include "xmalloc.h"
#include <stdlib.h>
#include <string.h>

struct clgstr {
        size_t length;          /* Length of string. */
        size_t allocated;       /* Allocated space in string buffer. */
        char *string;           /* String buffer. */
};

struct clgstr *clgstr_create(size_t pxreallocate)
{
        struct clgstr *gs = (struct clgstr *) xmalloc(sizeof(struct clgstr));
        gs->string = (char *) xmalloc(sizeof(char) * pxreallocate);
        gs->allocated = pxreallocate;
        gs->length = 0;
        return gs;
}

void clgstr_free(struct clgstr *gs)
{
        free(gs->string);
        free(gs);
}

void clgstr_append(struct clgstr *gs, const char *str)
{
        size_t add_len = strlen(str);

        /* Check to see if the appended string will fit in the current buffer,
           otherwise xreallocate a larger buffer with enough space and some room to grow. */
        if (gs->allocated < gs->length + add_len + 1) {
                size_t new_size = (gs->allocated + add_len + 1) * 1.5;
                gs->string = (char *) xrealloc(gs->string, new_size);
                gs->allocated = new_size;
        }
        /* Copy the string to append into the buffer and add a terminating zero. */
        memcpy(gs->string + gs->length, str, add_len);
        gs->length += add_len;
        gs->string[gs->length] = '\0';
}

char *clgstr_string(struct clgstr *gs)
{
        return gs->string;
}

size_t clgstr_length(struct clgstr * gs)
{
        return gs->length;
}
