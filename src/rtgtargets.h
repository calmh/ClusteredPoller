#ifndef _RTGTARGETS_H
#define _RTGTARGETS_H

#include "rtgconf.h"

#ifdef __cplusplus
extern "C" {
#endif

        /* Holds query instructions for one row (table+id). */
        typedef struct {
                char* oid;
                char* table;
                unsigned id;
                unsigned bits;
                unsigned long long speed;
        } queryrow;

        queryrow* queryrow_create();
        void queryrow_free(queryrow* row);

        /* Holds query instructions for one host. */
        typedef struct {
                char* host;
                char* community;
                int snmpver;

                queryrow** rows;
                unsigned nrows;
                unsigned allocated_rowspace;
        } queryhost;

        queryhost* queryhost_create();
        void queryhost_free(queryhost* host);

// Holds information from targets.cfg.
        typedef struct {
                queryhost** hosts;
                unsigned nhosts;
                unsigned allocated_space;
                unsigned ntargets;
        } rtgtargets;

        rtgtargets* rtgtargets_create();
        rtgtargets* rtgtargets_parse(const char* filename, const rtgconf* config);
        void rtgtargets_free(rtgtargets* targets);

#ifdef __cplusplus
}
#endif

#endif
