#include "util.h"
#include "globals.h"
#include "monitor.h"
#include "poller.h"
#include "database.h"
#include "version.h"

#include <iostream>
using namespace std;

void help();

// Setup and initialization.

// Display usage.
void help()
{
        cerr << "clpoll " << CLPOLL_VERSION << endl;
        cerr << " -c <file>   Specify configuration file [" << rtgconf << "]" << endl;
        cerr << " -D          Don't detach, run in foreground" << endl;
        cerr << " -d          Disable database inserts" << endl;
        cerr << " -t <file>   Specify target file [" << targets << "]" << endl;
        cerr << " -v          Increase verbosity" << endl;
        cerr << " -z          Database zero delta inserts" << endl;
        cerr << " -ql <num>   Maximum database queue length [" << max_queue_length << "]" << endl;
        cerr <<  " Copyright (c) 2009-2010 Jakob Borg" << endl;
}

void run_threads()
{
        // Calculate number of database writers needed. This is just a guess.
        unsigned num_dbthreads = config.threads / 8;
        num_dbthreads = num_dbthreads ? num_dbthreads : 1;

        if (verbosity >= 1)
                cerr << "Starting " << config.threads << " poller threads." << endl;
        Poller pollers(config.threads);
        pollers.start();

        if (verbosity >= 1)
                cerr << "Starting " << num_dbthreads << " database threads." << endl;
        Database database_threads(num_dbthreads);
        database_threads.start();

        if (verbosity >= 1)
                cerr << "Starting monitor thread." << endl;
        Monitor monitor;
        monitor.start();

        pollers.join_all();
        database_threads.join_all();
        monitor.join_all();
}

#ifndef TESTSUITE
// Parse command line, load caonfiguration and start threads.
int main (int argc, char* const argv[])
{
        if (argc < 2) {
                help();
                exit(0);
        }
        for (int i = 1; i < argc; i++) {
                string arg = string(argv[i]);
                if (arg == "-v")
                        verbosity++;
                else if (arg == "-D")
                        detach = 0;
                else if (arg == "-d")
                        use_db = 0;
                else if (arg == "-z")
                        allow_db_zero = 1;
                else if (arg == "-h") {
                        help();
                        exit(0);
                } else if (arg == "-c") {
                        i++;
                        rtgconf = string(argv[i]);
                } else if (arg == "-t") {
                        i++;
                        targets = string(argv[i]);
                } else if (arg == "-ql") {
                        i++;
                        max_queue_length = atoi(argv[i]);
                }
        }

        // Read rtg.conf
        config = RTGConf(rtgconf);
        // Read targets.cfg
        hosts = RTGTargets(targets, config);

        if (hosts.size() == 0) {
                cerr << "No hosts, so nothing to do." << endl;
                exit(-1);
        }

        // Allocate result cache for the number of hosts in targets.cfg
        cache = vector<ResultCache>(hosts.size());

        if (verbosity >= 1)
                cerr << "Polling every " << config.interval << " seconds." << endl;

        if (detach)
                daemonize();

        run_threads();
        return 0;
}
#endif
