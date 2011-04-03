//
//  clgstr.c
//  ClusteredPoller
//
//  Created by Jakob Borg on 2011-03-31.
//  Copyright 2011 Nym Networks. All rights reserved.
//

#include <stdlib.h>
#include <string.h>

#include "clgstr.h"

struct clgstr *clgstr_create(size_t preallocate) {
        struct clgstr *gs = (struct clgstr *) malloc(sizeof(struct clgstr));
        gs->string = (char *) malloc(sizeof(char) * preallocate);
        gs->allocated = preallocate;
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
        if (gs->allocated < gs->length + add_len + 1) {
                size_t new_size = (gs->allocated + add_len + 1) * 1.5;
                gs->string = (char *) realloc(gs->string, new_size);
                gs->allocated = new_size;
        }

        memcpy(gs->string + gs->length, str, add_len);
        gs->length += add_len;
        gs->string[gs->length] = '\0';
}

