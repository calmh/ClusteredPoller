#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <unistd.h>
#include <ctype.h>

#include "clbuf.h"
#include "util.h"
#include "globals.h"
#include "monitor.h"
#include "poller.h"
#include "database.h"
#include "version.h"
#include "multithread.h"
#include "rtgtargets.h"

void help();
void sighup_handler(int signum);
void sigterm_handler(int signum);

void help()
{
        fprintf(stderr, "\n");
        fprintf(stderr, "clpoll %s Copyright (c) 2009-2011 Jakob Borg\n", CLPOLL_VERSION);
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
        fprintf(stderr, " -T <num>    Number of poller threads per database thread [%d]\n", DEFAULT_DBTHREADS_DIVISOR);
        fprintf(stderr, " -Q <num>    Maximum database queue length [%d]\n", DEFAULT_QUEUE_LENGTH);
        fprintf(stderr, "\n");
}

void run_threads(struct rtgtargets *targets, struct rtgconf *config)
{
        thread_stop_requested = 0;
        active_threads = 0;
        stat_inserts = 0;
        stat_queries = 0;
        stat_iterations = 0;

        // Calculate number of database writers needed.
        unsigned num_dbthreads = config->threads / dbthreads_divisor;
        num_dbthreads = num_dbthreads ? num_dbthreads : 1;

        queries = clbuf_create(max_queue_length);

        cllog(1, "Starting %d poller threads.", config->threads);
        struct mt_threads *poller_threads = mt_threads_create(config->threads);
        unsigned i;
        for (i = 0; i < config->threads; i++) {
                struct poller_ctx *ctx = (struct poller_ctx *)malloc(sizeof(struct poller_ctx));
                ctx->targets = targets;
                poller_threads->contexts[i].param = ctx;
        }
        mt_threads_start(poller_threads, poller_run);

        cllog(1, "Starting %d database threads.", num_dbthreads);
        struct mt_threads *database_threads = mt_threads_create(num_dbthreads);
        for (i = 0; i < num_dbthreads; i++) {
                struct database_ctx *ctx = (struct database_ctx *)malloc(sizeof(struct database_ctx));
                ctx->config = config;
                database_threads->contexts[i].param = ctx;
        }
        mt_threads_start(database_threads, database_run);

        cllog(1, "Starting monitor thread.");
        struct mt_threads *monitor_threads = mt_threads_create(1);
        struct monitor_ctx *ctx = (struct monitor_ctx *)malloc(sizeof(struct monitor_ctx));
        ctx->interval = config->interval;
        ctx->targets = targets;
        monitor_threads->contexts[0].param = ctx;
        mt_threads_start(monitor_threads, monitor_run);

        mt_threads_join(database_threads);
        mt_threads_join(poller_threads);
        mt_threads_join(monitor_threads);

        clbuf_free(queries);
        queries = 0;

        for (i = 0; i < config->threads; i++)
                free(poller_threads->contexts[i].param);
        mt_threads_free(poller_threads);

        for (i = 0; i < num_dbthreads; i++)
                free(database_threads->contexts[i].param);
        mt_threads_free(database_threads);

        free(monitor_threads->contexts[0].param);
        mt_threads_free(monitor_threads);
}

#ifndef TESTSUITE
int main (int argc, char *const argv[])
{
        if (argc < 2) {
                help();
                exit(-1);
        }

        int c;
        while ((c = getopt (argc, argv, "c:dDt:T:vzQ:")) != -1) {
                switch (c) {
                case 'c':
                        rtgconf_file = optarg;
                        break;
                case 'd':
                        use_db = 0;
                        break;
                case 'D':
                        detach = 0;
                        break;
                case 't':
                        targets_file = optarg;
                        break;
                case 'v':
                        verbosity++;
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
                case 'z':
                        allow_db_zero = 1;
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
#endif

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
