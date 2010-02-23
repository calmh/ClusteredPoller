#ifndef _UTIL_H
#define _UTIL_H

#include <stdlib.h>
#include "types.h"

// See util.cpp for comments.

void daemonize(void);
std::string no_semi(std::string token);
template <typename Iter> void range_tolower (Iter beg, Iter end);
void string_tolower (std::string & str);

#endif
