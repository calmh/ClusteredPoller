#ifndef POLLER_H_
#define POLLER_H_

#include <time.h>

struct rtgtargets;
struct queryhost;

// Thread context (parameters) for the poller threads.
struct poller_ctx {
        struct rtgtargets *targets;
};

// Get all inserts for the specified host for this polling interval.
// Will cause a lot of SNMP queries etc to happen.
struct clinsert **get_clinserts(struct queryhost *host);

void *poller_run(void *param);

#endif
