//
//  clgstr.h
//  ClusteredPoller
//
//  Created by Jakob Borg on 2011-03-31.
//  Copyright 2011 Nym Networks. All rights reserved.
//

#ifndef GSTRING_H
#define GSTRING_H

struct clgstr {
        size_t length;
        size_t allocated;
        char *string;
};

struct clgstr *clgstr_create(size_t preallocate);
void clgstr_free(struct clgstr *gs);
void clgstr_append(struct clgstr *gs, const char *str);

#endif
