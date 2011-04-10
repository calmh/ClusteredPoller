//
//  clgstr.h
//  ClusteredPoller
//
//  Created by Jakob Borg on 2011-03-31.
//  Copyright 2011 Nym Networks. All rights reserved.
//

#ifndef GSTRING_H
#define GSTRING_H

// An automatically growing string object.
struct clgstr {
        size_t length;
        size_t allocated;
        char *string;
};

// Create a new clgstr object.
struct clgstr *clgstr_create(size_t preallocate);

// Free a clgstr object.
void clgstr_free(struct clgstr *gs);

// Append a char* to the clgstr.
void clgstr_append(struct clgstr *gs, const char *str);

// Get a pointer to the current string.
// This is a pointer to private memory, and should be strdup:ed if it
// is to be modified or stored for a long time.
void clgstr_append(struct clgstr *gs, const char *str);

#endif
