#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "rtgconf.h"

rtgconf *rtgconf_create(const char *filename)
{
        FILE *fileptr = fopen(filename, "rb");
        /* !!! This might fail. Handle it. */
        char buffer[513];
        char *line;
        rtgconf *conf = (rtgconf *) malloc(sizeof(rtgconf));
        while ((line = fgets(buffer, 512, fileptr))) {
                /* Lowercase string. */
		int i;
                for (i = 0; line[i] != 0; i++)
                        line[i] = tolower(line[i]);

                /* Terminate line at first comment character. */
                char *comment_begin = strchr(line, '#');
                if (comment_begin)
                        *comment_begin = '\0';

                /* Ignore empty lines. */
                if (strlen(line) == 0)
                        continue;

                const char *sep = " \t\n";
                char *token = strtok(line, sep);
                if (token) {
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

void rtgconf_free(rtgconf *config)
{
        free(config->dbhost);
        free(config->dbuser);
        free(config->dbpass);
        free(config->database);
        free(config);
}

