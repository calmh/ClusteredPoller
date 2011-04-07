#ifndef POLLER_H_
#define POLLER_H_

struct rtgtargets;

struct poller_ctx {
        struct rtgtargets *targets;
};

void *poller_run(void *param);

#endif                          /* POLLER_H_ */
