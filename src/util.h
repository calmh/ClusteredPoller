#ifndef _UTIL_H
#define _UTIL_H

#include <stdlib.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

// See util.cpp for comments.

        void daemonize(void);
        void cllog(int level, const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif
