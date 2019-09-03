#ifndef _gl_log_h
#define _gl_log_h 1



                            /*### @+prefix ************************************/
#include <stdio.h>
#include <stdlib.h>
#ifdef __cplusplus
    #include <cstdarg>
#else
    #include <stdarg.h>
#endif
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>

#include "umstypes.h"

#define GL_LEVEL_ERROR  1
#define GL_LEVEL_DEBUG  2
#define GL_LEVEL_TRACE  10
                            /*### @-prefix ************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*
** Get task id for process/thread
*/
extern pid_t gettid();




/* ----------------------------------------------------------------------
** Initialize the logger
**----------------------------------------------------------------------
**
** Initialize the gut log system which creates default logs of the format
**
** <timestamp> [ <program-name>(<thread/process ID>) <module>=<level> <FILENAME:FILENO> ] <message>
**
** Parameters:
**
** prog:
**    set the string that is used for the <program-name> field above. This is
**    also one of the fields used in the userconf file for limiting which logs
**    to display
**
** name:
**    (C++ only) allows the user to set another name for the logging. If NULL
**    then nothing extra is added. If non-NULL then the specified string is
**    appended after the (<thread/process ID>)
**
**    For example:
**        <timestamp> [ <program-name>(<thread/process ID>).<NAME>
**
** filename:
**     (C++ only) allows the user to choose the log file to write to. If NULL
**     is specified then the default file "/usr/BWhttpd/logs/ims.log" is used.
**
** addThreadName:
**     (C++ only) when true, it causes the logging threads name to be placed in
**     the gut log output. The logging thread name follows the optional name
**     parameter (see above).
**
**     For example:
**        <timestamp> [ <program-name>(<thread/process ID>).<NAME>:<THREADNAME>
**     or
**        <timestamp> [ <program-name>(<thread/process ID>):<THREADNAME>
**
**     depending upon if the <NAME> parameter is used
*/
extern void gut_init_logger(
    const char* prog,
#ifdef __cplusplus
    const char* name,
    const char* filename = NULL,
    bool addThreadName   = false);
#else
    const char* name);
#endif


/*----------------------------------------------------------------------
 * Clear the gut logger so it can be initialized again
 *----------------------------------------------------------------------
 * This clears the gut log's initialization so that it can be initialized
 * again. This is necessary for gut logging errors after a fork scenario,
 * before a new exec is done. Invoking this method will cause the the current
 * gut log file descriptor to be closed (if opened), and for the existing gut
 * log thread and allocated memory to be release so we can restart it all with
 * another gut_init_logger() call.
 */
extern void gut_clear_logger_init();

/*
** Log a message.
*/
extern void vgut_log(const char*    file,
                     int            line,
                     const char*    module,
                     uint32_t       level,
                     const char*    fmt,
                     va_list        ap);

extern void gut_log(
    const char* file,
    int line,
    const char* module,
    uint32_t level,
    const char* fmt,
    ...) __attribute__((format (printf,5,6)));

/*
** (Re)fetch log threshold parameters.
*/
extern void gut_reset_logger(void);
extern void gut_log_enable(void);
extern void gut_log_disable(void);

#define GUT_MODULE    "GL_UTIL"
#define GUT_BASE      __FILE__, __LINE__, GUT_MODULE
#define GUT_ALWAYS    GUT_BASE, 0
#define GUT_DEBUG(_L) GUT_BASE, GL_LEVEL_DEBUG + _L
#define GUT_ERROR     GUT_BASE, GL_LEVEL_ERROR
#define GUT_TRACE     GUT_BASE, GL_LEVEL_TRACE


#ifdef __cplusplus
}
#endif

#endif /* !_gl_log_h */


/* ==================== editors ====================== */

