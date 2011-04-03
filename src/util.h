#ifndef _UTIL_H
#define _UTIL_H

#include <stdlib.h>
#include <stdarg.h>

// See util.cpp for comments.

void daemonize(void);
void cllog(int level, const char *format, ...);

#endif
