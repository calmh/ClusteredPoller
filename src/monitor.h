#ifndef MONITOR_H_
#define MONITOR_H_

#include "multithread.h"

#ifdef __cplusplus
extern "C" {
#endif

        typedef struct {
                unsigned interval;
        } monitor_ctx;

        void *monitor_run(void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* MONITOR_H_ */
