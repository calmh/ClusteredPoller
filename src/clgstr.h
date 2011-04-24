//
//  ClusteredPoller
//
//  Created by Jakob Borg.
//  Copyright 2011 Nym Networks. See LICENSE for terms.
//

#ifndef GSTRING_H
#define GSTRING_H

#include <stddef.h>

/// @file clgstr.h
/// An automatically growing string object.

struct clgstr;

/// Create a new clgstr object.
/// @param pxreallocate The size to reserve space for from the beginning.
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

/// Get the length of the current string.
/// @param gs The clgstr object.
/// @return The length of the current string.
size_t clgstr_length(struct clgstr *gs);

#endif
