//
//  gstring.h
//  ClusteredPoller
//
//  Created by Jakob Borg on 2011-03-31.
//  Copyright 2011 Nym Networks. All rights reserved.
//

#ifndef GSTRING_H
#define GSTRING_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

        typedef struct {
                size_t length;
                size_t allocated;
                char *string;
        } gstr;

        gstr *gstr_create(size_t preallocate);
        void gstr_free(gstr *gs);
        void gstr_append(gstr *gs, const char *str);

#ifdef __cplusplus
}
#endif

#endif