//
//  ClusteredPoller
//
//  Created by Jakob Borg.
//  Copyright 2011 Nym Networks. See LICENSE for terms.
//

#ifndef RTGCONF_H
#define RTGCONF_H

/// @file rtgconf.h RTG.conf file parser

/// Object representing configuration settings found in rtg.conf, and extras specified on the command line.
struct rtgconf {
        // Standard RTG
        unsigned interval;      ///< The polling interval, in seconds.
        unsigned threads;       ///< The number of poller threads to use.
        char *dbhost;           ///< The database host.
        char *database;         ///< The database name.
        char *dbuser;           ///< The database user.
        char *dbpass;           ///< The database password.
        // Extended
        int use_db;             ///< False if we should not connect to the database.
        int use_rate_column;    ///< True if we should use the new schema with rate column.
        int allow_db_zero;      ///< True if we should insert zero rates in the database.
        unsigned num_dbthreads; ///< The number of database threads.
        unsigned max_db_queue;  ///< The maximum database queue depth.
};

/// Create an rtgconf object, parsed from the specified file.
/// @param filename The file to read.
/// @return A new rtgconf object.
struct rtgconf *rtgconf_create(const char *filename);

/// Verify that the settings are reasonably sane.
/// @param config The configuration to verify.
/// @return Nonzero if the configuration is OK.
int rtgconf_verify(struct rtgconf *config);

/// Free an rtgconf object.
/// @param config The object to free.
void rtgconf_free(struct rtgconf *config);

#endif // RTGCONF.H
