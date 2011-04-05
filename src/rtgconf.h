#ifndef _RTGCONF_H
#define _RTGCONF_H

struct rtgconf {
        unsigned interval;
        unsigned threads;
        char *dbhost;
        char *database;
        char *dbuser;
        char *dbpass;
};

struct rtgconf *rtgconf_create(const char *filename);
int rtgconf_verify(struct rtgconf *config);
void rtgconf_free(struct rtgconf *config);

#endif
