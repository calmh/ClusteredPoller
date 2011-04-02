#ifndef MONITOR_H_
#define MONITOR_H_

#include "multithread.h"
#include "rtgtargets.h"

typedef struct {
        unsigned interval;
        rtgtargets *targets;
} monitor_ctx;

void *monitor_run(void *ptr);

#endif /* MONITOR_H_ */
