//
//  ClusteredPoller
//
//  Created by Jakob Borg.
//  Copyright 2011 Nym Networks. See LICENSE for terms.
//

#ifndef GSTRING_H
#define GSTRING_H

/// @file clgstr.h
/// An automatically growing string object.

/// An automatically growing string object.
/// @see clgstr_create
/// @see clgstr_free
/// @see clgstr_append
/// @see clgstr_string
struct clgstr {
        size_t length;
        size_t allocated;
        char *string;
};

/// Create a new clgstr object.
/// @param preallocate The size to reserve space for from the beginning.
/// @return A new clgstr object.
struct clgstr *clgstr_create(size_t preallocate);

/// Free a clgstr object.
/// @param gs The clgstr object.
void clgstr_free(struct clgstr *gs);

/// Append a string to the clgstr.
/// @param gs The clgstr object.
/// @param str The string to append to the clgstr object. The string is copied.
void clgstr_append(struct clgstr *gs, const char *str);

/// Get a pointer to the current string.
/// @param gs The clgstr object.
/// @return The current string. This is a pointer to private memory, and should be strdup:ed if it is to be modified or stored for a long time.
char *clgstr_string(struct clgstr *gs);

#endif
