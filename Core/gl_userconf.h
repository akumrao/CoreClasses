/*------------------------------------------------------------------
 * May 2010, Arvind Umrao
 *
 * Copyright (c) 2010 byArvind Umrao.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#ifndef __GL_USERCONF_H__
#define __GL_USERCONF_H__

#ifdef __cplusplus
extern "C" {
#endif


// Returns user_config file name.  Don't delete memory returned
extern const char *gut_get_user_config_filename(void);


// debug functions
extern void gut_user_config_dump(void);


/*
 * Returns value of key if present in file, else returns NULL.
 * DO NOT DELETE or MODIFY the memory that is returned
 */
extern const char *gut_get_user_config_value(char *key);


// refresh cache of user_config (forces file to be re-read)
extern void gut_refresh_user_config(void);



#ifdef __cplusplus
}
#endif

#endif
