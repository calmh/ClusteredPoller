#ifndef _RTGCONF_H
#define _RTGCONF_H

#ifdef __cplusplus
extern "C" {
#endif

        /* Holds information from rtg.conf. */
        typedef struct {
                unsigned interval;
                unsigned threads;
                char *dbhost;
                char *database;
                char *dbuser;
                char *dbpass;
        } rtgconf;

        rtgconf *rtgconf_create(const char *filename);
        void rtgconf_free(rtgconf *config);

#ifdef __cplusplus
}
#endif

#endif
