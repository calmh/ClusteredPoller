#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>

#include "rtgtargets.h"
#include "rtgconf.h"
#include "util.h"

struct queryhost *read_host(FILE *fileptr, char *host_name, const struct rtgconf *conf);
struct queryrow *read_row(FILE *fileptr, char *oid, const struct rtgconf *conf);
int check_for_duplicate(struct queryhost *host, struct queryrow *row);
struct rtgtargets *read_new_style_targets(const char *filename, const struct rtgconf *conf);
struct rtgtargets *read_old_style_targets(const char *filename, const struct rtgconf *conf);
char *strtolower(char *str);
char *strclean(char *str);
char *strunc(char *str);
void queryhost_push_row(struct queryhost *host, struct queryrow *row);

struct rtgtargets *rtgtargets_create()
{
        struct rtgtargets *targets = (struct rtgtargets *) malloc(sizeof(struct rtgtargets));
        targets->nhosts = 0;
        targets->hosts = (struct queryhost **) malloc(sizeof(struct queryhost *) * 8);
        targets->allocated_space = 8;
        targets->ntargets = 0;
        targets->next_host = 0;
        pthread_mutexattr_t mta;
        pthread_mutexattr_init(&mta);
        pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&targets->next_host_lock, &mta);
        return targets;
}

void rtgtargets_free(struct rtgtargets *targets)
{
        int i;
        for (i = 0; i < targets->nhosts; i++)
                queryhost_free(targets->hosts[i]);
        free(targets->hosts);
        free(targets);
}

struct rtgtargets *rtgtargets_parse(const char *filename, const struct rtgconf *conf)
{
        struct rtgtargets *targets = read_new_style_targets(filename, conf);
        if (targets->nhosts == 0) {
                rtgtargets_free(targets);
                targets = read_old_style_targets(filename, conf);
        }
        cllog(0, "Read %d targets in %d hosts.", targets->ntargets, targets->nhosts);
        return targets;
}

void rtgtargets_push_host(struct rtgtargets *targets, struct queryhost *host)
{
        if (targets->nhosts == targets->allocated_space) {
                unsigned new_size = targets->allocated_space * 1.5;
                targets->hosts = (struct queryhost **) realloc(targets->hosts, sizeof(struct queryhost *) * new_size);
                targets->allocated_space = new_size;
        }
        targets->hosts[targets->nhosts++] = host;
        targets->ntargets += host->nrows;
}

struct rtgtargets *read_new_style_targets(const char *filename, const struct rtgconf *conf)
{
        struct rtgtargets *targets = rtgtargets_create();
        FILE *fileptr = fopen(filename, "rb");
        if (!fileptr)
                return targets;

        char token[513];
        while (fscanf(fileptr, " %512s", token) == 1) {
                strclean(strtolower(token));

                /* In case of comment, ignore up to end of line. */
                if (token[0] == '#') {
                        int result = fscanf(fileptr, "%*[^\n]");
                        (void)result;
                        continue;
                }

                if (!strcmp(token, "host")) {
                        int result = fscanf(fileptr, " %512s", token);
                        (void)result;
                        char *host_name = strdup(token);
                        struct queryhost *host = read_host(fileptr, host_name, conf);
                        rtgtargets_push_host(targets, host);
                }
        }
        fclose(fileptr);

        return targets;
}

struct queryhost *read_host(FILE *fileptr, char *host_name, const struct rtgconf *conf)
{
        struct queryhost *host = queryhost_create();
        host->host = host_name;
        char token[513];
        while (fscanf(fileptr, " %512s", token) == 1) {
                strclean(strtolower(token));

                int result; /* Unused */
                /* In case of comment, ignore up to end of line. */
                if (token[0] == '#') {
                        result = fscanf(fileptr, "%*[^\n]");
                        (void)result;
                        continue;
                }

                char buffer[129];
                if (!strcmp(token, "community")) {
                        result = fscanf(fileptr, " %128s", buffer);
                        host->community = strdup(strclean(buffer));
                } else if (!strcmp(token, "snmpver")) {
                        result = fscanf(fileptr, " %d", &host->snmpver);
                } else if (!strcmp(token, "target")) {
                        result = fscanf(fileptr, " %128s", buffer);
                        char *oid = strdup(strclean(buffer));
                        struct queryrow *row = read_row(fileptr, oid, conf);
                        if (!check_for_duplicate(host, row))
                                queryhost_push_row(host, row);
                        else
                                queryrow_free(row);
                } else if (!strcmp(token, "}")) {
                        break;
                }
                (void)result;
        }
        return host;
}

struct queryhost *queryhost_create()
{
        struct queryhost *host = (struct queryhost *) malloc(sizeof(struct queryhost));
        host->host = "<uninitialized>";
        host->community = "<uninitialized>";
        host->snmpver = 0;
        host->nrows = 0;
        host->rows = (struct queryrow **) malloc(sizeof(struct queryrow *) * 8);
        host->allocated_rowspace = 8;
        return host;
}

void queryhost_free(struct queryhost *host)
{
        int i;
        for (i = 0; i < host->nrows; i++)
                queryrow_free(host->rows[i]);
        free(host->host);
        free(host->community);
        free(host);
}

struct queryrow *queryrow_create()
{
        struct queryrow *row = (struct queryrow *) malloc(sizeof(struct queryrow));
        row->oid = "<uninitialized>";
        row->table = "<uninitialized>";
        row->id = 0;
        row->bits = 0;
        row->speed = 0;
        row->cached_time = 0;
        row->cached_counter = 0;
        return row;
}

void queryrow_free(struct queryrow *row)
{
        free(row->oid);
        free(row->table);
        free(row);
}

void queryhost_push_row(struct queryhost *host, struct queryrow *row)
{
        if (host->nrows == host->allocated_rowspace) {
                unsigned new_size =  host->allocated_rowspace * 1.5;
                host->rows = (struct queryrow **) realloc(host->rows, sizeof(struct queryrow *) * new_size);
                host->allocated_rowspace = new_size;
        }
        host->rows[host->nrows++] = row;
}

struct queryrow *read_row(FILE *fileptr, char *oid, const struct rtgconf *conf)
{
        struct queryrow *row = queryrow_create();
        row->oid = oid;
        char token[513];
        while (fscanf(fileptr, " %512s", token) == 1) {
                strclean(strtolower(token));

                int result;
                /* In case of comment, ignore up to end of line. */
                if (token[0] == '#') {
                        result = fscanf(fileptr, "%*[^\n]");
                        continue;
                }

                char buffer[129];
                if (!strcmp(token, "bits")) {
                        result = fscanf(fileptr, " %d", &row->bits);
                } else if (!strcmp(token, "table")) {
                        result = fscanf(fileptr, " %128s", buffer);
                        row->table = strdup(strclean(buffer));
                } else if (!strcmp(token, "id")) {
                        result = fscanf(fileptr, " %d", &row->id);
                } else if (!strcmp(token, "speed")) {
                        unsigned long long max_counter_diff;
                        result = fscanf(fileptr, " %llu", &max_counter_diff);
                        if (row->bits == 0)
                                row->speed = max_counter_diff;
                        else
                                row->speed = max_counter_diff / conf->interval;
                } else if (!strcmp(token, "}")) {
                        break;
                }
                (void) result;
        }
        return row;
}

char *strtolower(char *str)
{
        /* Lowercase string. */
        int i;
        for (i = 0; str[i] != 0; i++)
                str[i] = tolower(str[i]);
        return str;
}

char *strclean(char *str)
{
        /* Remove (end the string at) any semicolon. */
        char *semicolon = strchr(str, ';');
        if (semicolon)
                *semicolon = '\0';

        /* !!! Should also remove whitespace at both ends. */

        return str;
}

char *strunc(char *str)
{
        /* Remove (end the string at) any semicolon. */
        char *comment = strchr(str, '#');
        if (comment)
                *comment = '\0';

        /* !!! Should also remove whitespace at both ends. */

        return str;
}

int check_for_duplicate(struct queryhost *host, struct queryrow *row)
{
        unsigned i;
        for (i = 0; i < host->nrows; i++) {
                struct queryrow *it_row = host->rows[i];
                if (!strcmp(it_row->oid, row->oid)) {
                        cllog(0, "WARNING: Host %s OID %s is a duplicate. Ignoring.", host->host, row->oid);
                        return 1;
                }
                if (!strcmp(it_row->table, row->table) && it_row->id == row->id) {
                        cllog(0, "WARNING: Host %s table %s id %d is a duplicate. Ignoring.", host->host, row->table, row->id);
                        return 1;
                }
        }
        return 0;
}

struct rtgtargets *read_old_style_targets(const char *filename, const struct rtgconf *conf)
{
        struct rtgtargets *targets = rtgtargets_create();
        FILE *fileptr = fopen(filename, "rb");
        if (!fileptr)
                return targets;

        char linebuffer[257];
        char *line;
        struct queryhost *current_host = NULL;
        while ((line = fgets(linebuffer, 256, fileptr))) {
                strunc(line);
                if (strlen(line) == 0)
                        continue;

                char *tokens[256];
                const char *sep = "\t";
                tokens[0] = strtok(line, sep);
                int nbr_tokens = 1;
                while ((tokens[nbr_tokens] = strtok(NULL, sep)))
                        nbr_tokens++;

                if (nbr_tokens < 6)
                        continue;

                char *host = strdup(tokens[0]);
                char *oid = strdup(tokens[1]);
                int bits = atoi(tokens[2]);
                char *community = strdup(tokens[3]);
                char *table = strdup(tokens[4]);
                int id = atoi(tokens[5]);

                if (current_host != NULL && strcmp(current_host->host, host) != 0) {
                        /* Not the same host as previous row, so invalidate. */
                        current_host = NULL;
                }

                if (current_host == NULL) {
                        /* Look for an existing host. */
                        int i;
                        for (i = 0; i < targets->nhosts; i++) {
                                if (!strcmp(targets->hosts[i]->host, host)) {
                                        current_host = targets->hosts[i];
                                        break;
                                }
                        }
                }

                if (current_host == NULL) {
                        /* Create a new host. We lack data, so we assume SNMP version 2. */
                        struct queryhost *qhost = queryhost_create();
                        qhost->host = host;
                        qhost->community = community;
                        qhost->snmpver = 2;
                        rtgtargets_push_host(targets, qhost);
                        current_host = qhost;
                } else {
                        free(host);
                        free(community);
                }

                struct queryrow *row = queryrow_create();
                row->oid = oid;
                row->table = table;
                row->id = id;
                row->bits = bits;
                row->speed = (unsigned)10e9 / 8 / conf->interval;
                queryhost_push_row(current_host, row);
                targets->ntargets++;
        }

        fclose(fileptr);
        return targets;
}

struct queryhost *rtgtargets_next(struct rtgtargets *targets)
{
        struct queryhost *host = NULL;
        pthread_mutex_lock(&targets->next_host_lock);
        if (targets->next_host < targets->nhosts)
                host = targets->hosts[targets->next_host++];
        pthread_mutex_unlock(&targets->next_host_lock);
        return host;
}

void rtgtargets_reset_next(struct rtgtargets *targets)
{
        targets->next_host = 0;
}
