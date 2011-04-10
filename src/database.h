#ifndef DATABASE_H_
#define DATABASE_H_

struct rtgconf;

// Thread context (parameters) for the database threads.
struct database_ctx {
        struct rtgconf *config;
};

void *database_run(void *ptr);

#endif                          /* DATABASE_H_ */
