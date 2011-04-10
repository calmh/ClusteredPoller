#ifndef MONITOR_H_
#define MONITOR_H_

struct rtgtargets;
struct rtgconf;

// Thread context (parameters) for the monitor thread.
struct monitor_ctx {
        unsigned interval;      //
        struct rtgtargets *targets;
        struct rtgconf *config;
};

void *monitor_run(void *ptr);

#endif                          /* MONITOR_H_ */
