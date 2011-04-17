//
//  ClusteredPoller
//
//  Created by Jakob Borg.
//  Copyright 2011 Nym Networks. See LICENSE for terms.
//

#ifndef DATABASE_H
#define DATABASE_H

/// @file database.h
/// Database writer thread.

struct rtgconf;

/// Thread context (parameters) for the database threads.
struct database_ctx {
        struct rtgconf *config; ///< RTG.conf object.
};

/// Main loop for database thread.
/// @param ptr A pointer to a database_ctx object.
/// @return NULL
void *database_run(void *ptr);

#endif /* DATABASE_H */
