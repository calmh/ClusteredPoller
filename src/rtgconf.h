#ifndef _RTGCONF_H
#define _RTGCONF_H

/* Holds information from rtg.conf. */
struct rtgconf {
        unsigned interval;
        unsigned threads;
        char *dbhost;
        char *database;
        char *dbuser;
        char *dbpass;
};

struct rtgconf *rtgconf_create(const char *filename);
void rtgconf_free(struct rtgconf *config);

#endif
