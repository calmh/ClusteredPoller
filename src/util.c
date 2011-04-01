#include <sys/stat.h>
#include <stdio.h>
#include <syslog.h>
#include <unistd.h>

#include "globals.h"
#include "util.h"

// Detach from console. Common recipe.
void daemonize(void)
{
        pid_t pid, sid;

        if ( getppid() == 1 ) return;

        pid = fork();
        if (pid < 0) {
                exit(EXIT_FAILURE);
        }

        if (pid > 0) {
                exit(EXIT_SUCCESS);
        }

        umask(0);

        sid = setsid();
        if (sid < 0) {
                exit(EXIT_FAILURE);
        }

        if ((chdir("/")) < 0) {
                exit(EXIT_FAILURE);
        }

        FILE *ignored;
        ignored = freopen( "/dev/null", "r", stdin);
        ignored = freopen( "/dev/null", "w", stdout);
        ignored = freopen( "/dev/null", "w", stderr);
}

void cllog(int level, const char *format, ...)
{
        va_list ap;
        char buffer[128];
        va_start(ap, format);
        vsnprintf(buffer, 128, format, ap);
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
