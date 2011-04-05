#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>

#include "clbuf.h"
#include "cllog.h"
#include "globals.h"
#include "monitor.h"
#include "poller.h"
#include "database.h"
#include "version.h"
#include "multithread.h"
#include "rtgtargets.h"
#include "rtgconf.h"

void help();
void run_threads(struct rtgtargets *targets, struct rtgconf *config);
struct mt_threads *create_poller_threads(unsigned nthreads, struct rtgtargets *targets);
struct mt_threads *create_database_threads(unsigned nthreads, struct rtgconf *config);
struct mt_threads *create_monitor_thread(struct rtgtargets *targets, unsigned interval);
void free_threads_params(struct mt_threads *threads);
void sighup_handler(int signum);
void sigterm_handler(int signum);
void daemonize();

int main (int argc, char *const argv[])
{
        if (argc < 2) {
                help();
                exit(-1);
        }

        int c;
        while ((c = getopt (argc, argv, "c:dt:vzDOOT:Q:")) != -1) {
                switch (c) {
                case 'c':
                        rtgconf_file = optarg;
                        break;
                case 'd':
                        use_db = 0;
                        break;
                case 't':
                        targets_file = optarg;
                        break;
                case 'v':
                        verbosity++;
                        break;
                case 'z':
                        allow_db_zero = 1;
                        break;
                case 'D':
                        detach = 0;
                        break;
                case 'O':
                        use_rate_column = 0;
                        break;
                case 'Q':
                        max_queue_length = atoi(optarg);
                        if (max_queue_length < MIN_QUEUE_LENGTH) {
                                fprintf(stderr, "Error: minimum queue length is %d.\n", MIN_QUEUE_LENGTH);
                                help();
                                exit(-1);
                        }
                        break;
                case 'T':
                        dbthreads_divisor = atoi(optarg);
                        if (dbthreads_divisor < 1) {
                                fprintf(stderr, "Error: minimum 1 poller thread per database thread (more recommended).\n");
                                help();
                                exit(-1);
                        }
                        break;

                case '?':
                default:
                        help();
                        exit(-1);
                }
        }

        if (argc != optind) {
                help();
                exit(-1);
        }

        if (detach)
                daemonize();

        char *last_component = strrchr(argv[0], '/');
        if (last_component)
                last_component++;
        else
                last_component = argv[0];
        openlog(last_component, LOG_PID, LOG_USER);

        signal(SIGHUP, sighup_handler);
        signal(SIGTERM, sigterm_handler);

        while (!full_stop_requested) {
                // Read rtg.conf
                struct rtgconf *config = rtgconf_create(rtgconf_file);
                if (!config) {
                        cllog(0, "No configuration, so nothing to do.");
                        exit(-1);
                }

                // Read targets.cfg
                struct rtgtargets *targets = rtgtargets_parse(targets_file, config);

                if (targets->ntargets == 0) {
                        cllog(0, "No targets, so nothing to do.");
                        exit(-1);
                }

                cllog(1, "Polling every %d seconds.", config->interval);

                run_threads(targets, config);

                rtgconf_free(config);
                rtgtargets_free(targets);
        }
        return 0;
}

void help()
{
        fprintf(stderr, "\n");
        fprintf(stderr, "clpoll v%s Copyright (c) 2009-2011 Jakob Borg\n", CLPOLL_VERSION);
        fprintf(stderr, "\n");
        fprintf(stderr, "Legacy (rtgpoll compatible) options:\n");
        fprintf(stderr, " -c <file>   Specify configuration file [%s]\n", rtgconf_file);
        fprintf(stderr, " -d          Disable database inserts\n");
        fprintf(stderr, " -t <file>   Specify target file [%s]\n", targets_file);
        fprintf(stderr, " -v          Increase verbosity\n");
        fprintf(stderr, " -z          Database zero delta inserts\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Extended options:\n");
        fprintf(stderr, " -D          Don't detach, run in foreground\n");
        fprintf(stderr, " -O          Use old database schema, no `rate` column\n");
        fprintf(stderr, " -Q <num>    Maximum database queue length [%d]\n", DEFAULT_QUEUE_LENGTH);
        fprintf(stderr, " -T <num>    Number of poller threads per database thread [%d]\n", DEFAULT_DBTHREADS_DIVISOR);
        fprintf(stderr, "\n");
}

void run_threads(struct rtgtargets *targets, struct rtgconf *config)
{
        thread_stop_requested = 0;
        active_threads = 0;
        stat_inserts = 0;
        stat_queries = 0;
        stat_iterations = 0;

        unsigned num_dbthreads = config->threads / dbthreads_divisor;
        num_dbthreads = num_dbthreads ? num_dbthreads : 1;

        queries = clbuf_create(max_queue_length);

        struct mt_threads *poller_threads = create_poller_threads(config->threads, targets);
        struct mt_threads *database_threads = create_database_threads(num_dbthreads, config);
        struct mt_threads *monitor_thread = create_monitor_thread(targets, config->interval);

        mt_threads_join(database_threads);
        mt_threads_join(poller_threads);
        mt_threads_join(monitor_thread);

        free_threads_params(poller_threads);
        free_threads_params(database_threads);
        free_threads_params(monitor_thread);

        clbuf_free(queries);
        queries = 0;
}

struct mt_threads *create_poller_threads(unsigned nthreads, struct rtgtargets *targets) {
        cllog(1, "Starting %d poller threads.", nthreads);
        struct mt_threads *poller_threads = mt_threads_create(nthreads);
        unsigned i;
        for (i = 0; i < nthreads; i++) {
                struct poller_ctx *ctx = (struct poller_ctx *)malloc(sizeof(struct poller_ctx));
                ctx->targets = targets;
                poller_threads->contexts[i].param = ctx;
        }
        mt_threads_start(poller_threads, poller_run);
        return poller_threads;
}

struct mt_threads *create_database_threads(unsigned nthreads, struct rtgconf *config) {
        unsigned i;
        cllog(1, "Starting %d database threads.", nthreads);
        struct mt_threads *database_threads = mt_threads_create(nthreads);
        for (i = 0; i < nthreads; i++) {
                struct database_ctx *ctx = (struct database_ctx *)malloc(sizeof(struct database_ctx));
                ctx->config = config;
                database_threads->contexts[i].param = ctx;
        }
        mt_threads_start(database_threads, database_run);
        return database_threads;
}

struct mt_threads *create_monitor_thread(struct rtgtargets *targets, unsigned interval) {
        cllog(1, "Starting monitor thread.");
        struct mt_threads *monitor_threads = mt_threads_create(1);
        struct monitor_ctx *ctx = (struct monitor_ctx *)malloc(sizeof(struct monitor_ctx));
        ctx->interval = interval;
        ctx->targets = targets;
        monitor_threads->contexts[0].param = ctx;
        mt_threads_start(monitor_threads, monitor_run);
        return monitor_threads;
}

void free_threads_params(struct mt_threads *threads)
{
        unsigned i;
        for (i = 0; i < threads->nthreads; i++)
                free(threads->contexts[i].param);
        mt_threads_free(threads);
}

void sighup_handler(int signum)
{
        cllog(0, "Received SIGHUP. Shutting down threads and reinitializing...");
        thread_stop_requested = 1;
        pthread_mutex_lock(&global_lock);
        pthread_cond_broadcast(&global_cond);
        pthread_mutex_unlock(&global_lock);
}

void sigterm_handler(int signum)
{
        cllog(0, "Received SIGTERM. Shutting down...");
        thread_stop_requested = 1;
        full_stop_requested = 1;
        pthread_mutex_lock(&global_lock);
        pthread_cond_broadcast(&global_cond);
        pthread_mutex_unlock(&global_lock);
}

void daemonize()
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
