/*
 *  ClusteredPoller
 *
 *  Created by Jakob Borg.
 *  Copyright 2011 Nym Networks. See LICENSE for terms.
 */

#include "clinsert.h"

#include "xmalloc.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static struct clinsert *clinsert_create(char *table);

struct clinsert *clinsert_create(char *table)
{
        struct clinsert *insert = (struct clinsert *) xmalloc(sizeof(struct clinsert));
        insert->table = table;
        insert->values = (struct clinsert_value *) xmalloc(sizeof(struct clinsert_value) * 8);
        insert->allocated_space = 8;
        insert->nvalues = 0;
        return insert;
}

void clinsert_free(struct clinsert *insert)
{
        free(insert->values);
        free(insert);
}

void clinsert_push_value(struct clinsert *insert, unsigned id, unsigned long long counter, unsigned rate, time_t dtime)
{
        if (insert->nvalues == insert->allocated_space) {
                unsigned new_size = insert->allocated_space * 1.5;
                insert->values = (struct clinsert_value *) xrealloc(insert->values, sizeof(struct clinsert_value) * new_size);
                insert->allocated_space = new_size;
        }

        insert->values[insert->nvalues].id = id;
        insert->values[insert->nvalues].counter = counter;
        insert->values[insert->nvalues].rate = rate;
        insert->values[insert->nvalues].dtime = dtime;
        insert->nvalues++;
}

struct clinsert *clinsert_for_table(struct clinsert **inserts, char *table)
{
        int i;
        for (i = 0; i < MAX_TABLES && inserts[i]; i++) {
                if (!strcmp(inserts[i]->table, table)) {
                        return inserts[i];
                }
        }
        assert(i != MAX_TABLES);
        inserts[i] = clinsert_create(table);
        return inserts[i];
}

unsigned clinsert_count(struct clinsert **inserts)
{
        int count;
        for (count = 0; inserts[count] && count < MAX_TABLES; count++) ;
        return count;
}
