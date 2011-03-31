//
//  gstring.c
//  ClusteredPoller
//
//  Created by Jakob Borg on 2011-03-31.
//  Copyright 2011 Nym Networks. All rights reserved.
//

#include <string.h>
#include "gstring.h"

gstr* gstr_create(size_t preallocate)
{
        gstr* gs = (gstr*) malloc(sizeof(gstr));
        gs->string = (char*) malloc(sizeof(char) * preallocate);
        gs->allocated = preallocate;
        gs->length = 0;
        return gs;
}

void gstr_free(gstr* gs)
{
        free(gs->string);
        free(gs);
}

void gstr_append(gstr* gs, const char* str)
{
        size_t add_len = strlen(str);
        if (gs->allocated < gs->length + add_len + 1) {
                size_t new_size = (gs->allocated + add_len + 1) * 1.5;
                gs->string = (char*) realloc(gs->string, new_size);
                gs->allocated = new_size;
        }
        
        memcpy(gs->string + gs->length, str, add_len);
        gs->length += add_len;
        gs->string[gs->length] = '\0';
}

