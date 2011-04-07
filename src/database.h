#ifndef DATABASE_H_
#define DATABASE_H_

struct rtgconf;

struct database_ctx {
        struct rtgconf *config;
};

void *database_run(void *ptr);

#endif                          /* DATABASE_H_ */
