//
//  ClusteredPoller
//
//  Created by Jakob Borg.
//  Copyright 2011 Nym Networks. See LICENSE for terms.
//

#ifndef MONITOR_H
#define MONITOR_H

/// @file monitor.h
/// Monitor thread.

struct rtgtargets;
struct rtgconf;

/// Thread context (parameters) for the monitor thread.
struct monitor_ctx {
        struct rtgtargets *targets;     ///< The targets to process.
        struct rtgconf *config; ///< The configuration to use.
};

/// Main loop for monitor thread.
/// @param ptr A pointer to a monitor_ctx object.
/// @return NULL
void *monitor_run(void *ptr);

#endif /* MONITOR_H */
