#include <stdio.h>
#include <syslog.h>
#include <stdarg.h>
#include <pthread.h>

#include "globals.h"
#include "cllog.h"

void cllog(int level, const char *format, ...)
{
        va_list ap;
        char buffer[MAX_LINE_LENGTH];
        va_start(ap, format);
        vsnprintf(buffer, MAX_LINE_LENGTH, format, ap);
        va_end(ap);
        if (verbosity >= level) {
                pthread_mutex_lock(&cerr_lock);
                fprintf(stderr, "%s\n", buffer);
                pthread_mutex_unlock(&cerr_lock);
                if (level > 1)
                        syslog(LOG_DEBUG, "%s", buffer);
                else
                        syslog(LOG_INFO, "%s", buffer);
        }
}
