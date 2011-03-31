#ifndef POLLER_H_
#define POLLER_H_

#include "multithread.h"
#include "rtgtargets.h"

#ifdef __cplusplus
extern "C" {
#endif

        typedef struct {
                unsigned stride;
                rtgtargets* targets;
        } poller_ctx;

        void* poller_run(void* param);

#ifdef __cplusplus
}
#endif
#endif /* POLLER_H_ */
