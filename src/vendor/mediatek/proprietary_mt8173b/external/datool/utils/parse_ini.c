/*
 * Copyright (C) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "parse_ini.h"

/* read key_value from mtxxxx_dl_addr.ini */
char *getKeyValue(char *filename, char *section, char *key, char line[])
{
    char sectname[128];
    char *skey, *value;
    char delim[] = "=";
    bool flag = false;
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Error open file %s\n", filename);
        exit(0);
    }
    if (!strchr(section, '[')) {
        strcpy(sectname, "[");
        strcat(sectname, section);
        strcat(sectname, "]");
    } else {
        strcpy(sectname, section);
    }

    while (fgets(line, 128, fp) != NULL) {
        /* ignore line start with ';' */
        if (line[0] == ';') /* ';' is equal to 59 */
            continue;

        /* delete the newline: replace the newline token with NUL */
        value = strchr(line, '\n');
        *value = 0;

        if (!flag) {
            if (strcmp(sectname, line) == 0)
                flag = true;    /* find the section */
        } else {
            skey = strtok(line, delim);
            if (strcmp(skey, key) == 0) {
                fclose(fp);
                return strtok(NULL, delim);
            }
        }
    }

    fclose(fp);
    return NULL;
}

