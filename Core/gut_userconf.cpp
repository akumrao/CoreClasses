/****************************************************************

 *
 * File:                gut_userconf.c
 * Module:              util
 * Layer:               gl
 *
 * This file contains the routines to read/write modify etc a single
 * text user configuration file.
 *
 * Author:              Arvind Umrao <akumrao@yahoo.com>
 *
 * History:
 *
 ****************************************************************/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>

#include "gl_log.h"
#include "umstypes.h"
#include "gl_userconf.h"


#define UCONF_FILENAME          "userconf.txt"


struct u_config_str {
    char *key;      // key name
    char *value;    // key value
};
typedef struct u_config_str u_config_str_t;

/*
 * some constants
 */
#define UCONF_BUF_LEN           1024
#define UCONF_ARR_SZ            100

static u_config_str_t *user_config = NULL;
static int user_config_size = 0;
static int user_config_valid_entries = 0;
static const char *delims = ":";
static bool user_config_init = false;

/*
 * return the user config path and filename (full name)
 */
const char *gut_get_user_config_filename()
{
    static char fname[PATHSIZE];
    sprintf(fname,"%s/%s",CONFIG_PATH,UCONF_FILENAME);
    return(fname);
}

/*
 * From sysmon code
 */
char *gut_chop(char *l)
{
        char *k = l;

        // trim trailing spaces
        while (*l) l++;
        while (--l > k)
                if (isspace ((int) *l)) *l = '\0';
                else break;
        // trim leading spaces
        while (isspace ((int) *k)) k++;
        return (k);
}

char *gut_delete_trailing_comment(char *l)
{
    char *k = l;

    while (*l) {
        if (*l == '#') {
            *l = '\0';
            l++;
            break;
        }
        l++;
    }
    /* l is either pointing at a null char or past the start of a comment */
    while (*l) {
        *l = '\0';
        l++;
    }
    return (k);
}

/*
 * Read's the config file from disk and caches it in memory
 */
static void gut_user_config_read()
{
    FILE *fpi;
    char buf[UCONF_BUF_LEN];
    char *line;
    char *key, *value;
    int buf_idx = 0;
    struct stat statbuf;
    const char* fname = NULL;

    gut_log(GUT_TRACE, ">>>> enter gut_user_config_read()");

    if ( user_config != NULL )
        return;

    fname = gut_get_user_config_filename();
    if (stat(fname, &statbuf) < 0) {
        gut_log(GUT_ERROR, "stat(%s): %s\n", fname, strerror(errno));
        
        if ( (fpi = fopen(fname, "a+")) == NULL ) {
            gut_log(GUT_ERROR, "fopen(%s): %s\n", fname, strerror(errno));
            return;
        }
    }
    else
    if ( (fpi = fopen(fname, "r")) == NULL ) {
        gut_log(GUT_ERROR, "fopen(%s): %s\n", fname, strerror(errno));
        return;
    }

    user_config_size = UCONF_ARR_SZ;
    user_config = (u_config_str_t*)calloc(1, sizeof(u_config_str_t) * user_config_size);

    // read the entire file now
    while (fgets(buf, UCONF_BUF_LEN, fpi)) {
        line = gut_delete_trailing_comment(buf);
        line = gut_chop(line);
        // lines starting with a # are converted into null lines
        if (line[0] == '\0')
            continue;

        // first token should be the key
        key = strsep(&line,delims);
        if (key == NULL)
            continue;
        key = gut_chop(key);
        if (key[0] == '\0')
            continue;
        value = strsep(&line, delims);
        if (value == NULL)
            continue;
        value = gut_chop(value);
        if (value[0] == '\0')
            continue;
        user_config[buf_idx].key = strdup(key);
        user_config[buf_idx].value = strdup(value);
        user_config_valid_entries++;
        gut_log(GUT_DEBUG(1), "key=%s:value=%s", user_config[buf_idx].key, user_config[buf_idx].value);
        buf_idx++;
        if (buf_idx >= user_config_size) {
            user_config_size = user_config_size + UCONF_ARR_SZ;
            user_config = (u_config_str_t*)realloc(user_config,
                sizeof(u_config_str_t) * user_config_size);
        }
    }

    fclose(fpi);
    user_config_init = true;
    gut_log(GUT_TRACE, "<<<< exit gut_user_config_read()");
}

/*
 * for debug use
 */
void gut_user_config_dump()
{
    int x;

    for (x = 0; x < user_config_valid_entries; x++) {
        gut_log(GUT_DEBUG(1), "KEY <%s>:Value <%s>\n",
                user_config[x].key,
                user_config[x].value);
    }
}

/*
 * Return the value of the 'key' on success
 * returns NULL if not found
 */
const char *gut_get_user_config_value(char *key)
{
    int x;

    if ( !key )
        return NULL;

    // initialize on demand
    if ( !user_config )
        gut_user_config_read();

    // make sure it is initialized before continuing
    int msec = 10000000;
    while ( !user_config_init  && msec ) {
        msec -= 100000;
        usleep(100000);
    }
    if ( msec == 0 )
        return NULL;

    for (x = 0; x < user_config_valid_entries; x++) {
        if ( strcmp(user_config[x].key,key) == 0 )
            return user_config[x].value;
    }

    return NULL;
}

/*
 * used for resetting the cache that caches the user config file
 */
void gut_refresh_user_config()
{
    int x;

    user_config_init = false;
    for (x = 0; x < user_config_valid_entries; x++) {
        if ( !user_config[x].key )
            break;
        free(user_config[x].key);
        free(user_config[x].value);
    }
    if ( user_config )
        free(user_config);
    user_config = NULL;
    user_config_size = 0;
    user_config_valid_entries = 0;
}
