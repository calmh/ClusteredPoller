#ifndef DATABASE_H_
#define DATABASE_H_

#include "rtgconf.h"
#include "multithread.h"

#ifdef __cplusplus
extern "C" {
#endif

        typedef struct {
                rtgconf* config;
        } database_ctx;

        void* database_run(void* ptr);

#ifdef __cplusplus
}
#endif

#endif /* DATABASE_H_ */
