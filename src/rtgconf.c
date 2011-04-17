//
//  ClusteredPoller
//
//  Created by Jakob Borg.
//  Copyright 2011 Nym Networks. See LICENSE for terms.
//

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "rtgconf.h"
#include "cllog.h"

struct rtgconf *rtgconf_create(const char *filename)
{
        FILE *fileptr = fopen(filename, "rb");
        if (!fileptr) {
                cllog(0, "Couldn't open %s for reading.", filename);
                return NULL;
        }

        char buffer[513];
        char *line;

        struct rtgconf *conf = (struct rtgconf *) malloc(sizeof(struct rtgconf));
        conf->interval = 300;
        conf->threads = 4;
        conf->dbhost = NULL;
        conf->database = NULL;
        conf->dbuser = NULL;
        conf->dbpass = NULL;

        while ((line = fgets(buffer, 512, fileptr))) {
                /* Terminate line at first comment character. */
                char *comment_begin = strchr(line, '#');
                if (comment_begin)
                        *comment_begin = '\0';

                /* Ignore empty lines. */
                if (strlen(line) == 0)
                        continue;

                const char *sep = " \t\n";
                char *token = strtok(line, sep);
                /* Lowercase token. */
                if (token) {
                        int i;
                        for (i = 0; token[i] != 0; i++)
                                token[i] = tolower(token[i]);
                        if (!strcmp(token, "interval"))
                                conf->interval = atoi(strtok(NULL, sep));
                        else if (!strcmp(token, "db_host"))
                                conf->dbhost = strdup(strtok(NULL, sep));
                        else if (!strcmp(token, "db_database"))
                                conf->database = strdup(strtok(NULL, sep));
                        else if (!strcmp(token, "db_user"))
                                conf->dbuser = strdup(strtok(NULL, sep));
                        else if (!strcmp(token, "db_pass"))
                                conf->dbpass = strdup(strtok(NULL, sep));
                        else if (!strcmp(token, "threads"))
                                conf->threads = atoi(strtok(NULL, sep));
                }
        }
        fclose(fileptr);
        return conf;
}

int rtgconf_verify(struct rtgconf *config)
{
        if (!config->dbhost) {
                cllog(0, "Missing directive for database host in RTG config file.");
                return 0;
        }
        if (!config->database) {
                cllog(0, "Missing directive for database name in RTG config file.");
                return 0;
        }
        if (!config->dbuser) {
                cllog(0, "Missing directive for database user in RTG config file.");
                return 0;
        }
        return 1;
}

void rtgconf_free(struct rtgconf *config)
{
        free(config->dbhost);
        free(config->dbuser);
        free(config->dbpass);
        free(config->database);
        free(config);
}
