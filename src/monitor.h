#ifndef MONITOR_H_
#define MONITOR_H_

struct rtgtargets;

struct monitor_ctx {
        unsigned interval;
        struct rtgtargets *targets;
};

void *monitor_run(void *ptr);

#endif                          /* MONITOR_H_ */
