
#include "gl_log.h"
#include "gl_userconf.h"

                            /*### @+prefix ************************************/
#include <fcntl.h>
#include <pthread.h>
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <ctype.h>
#include <signal.h>
#include <syslog.h>
#include <time.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <errno.h>

#define GUT_INIT_BUFSIZE    10240
#define GUT_MAX_BUFSIZE     51200

#define GUT_LOGFILE "/usr/BWhttpd/logs/ims.log"
#define GUT_MAX_MODULES 64
                            /*### @-prefix ************************************/
                            /*### @struct GutLogLevel {{{2 ********************/

/*
** Log level by module.
*/
typedef struct GutLogLevel {
    char mModule[32];
    uint32_t mLevel;
} GutLogLevel;
                            /*### @struct GutLogLevel }}}2 ********************/
                            /*### @struct GutBuf {{{2 *************************/

/*
** Buffer used to pass messages from main program to logging thread.
*/
typedef struct GutBuf {
    char* mBuf;
    uint32_t mCapacity;
    uint32_t mLength;
} GutBuf;
                            /*### @struct GutBuf }}}2 *************************/

/*
** Function prototypes.
*/
static void shutdown_logger(void);
static int llcompare(const void*, const void*);
static bool log_allow(const char*, uint32_t);
static void* thread_main(void*);

/* Buffer accessed by users of interface. */
static GutBuf* sBuf;

/* Condition variable for signaling that buffer has contents. */
static pthread_cond_t sBufCond;

/* Mutex that protects buffer. */
static pthread_mutex_t sBufMutex;

/* Handle for file logger */
#define DEFAULT_SLOGFD   (-1)
static int sLogFd = DEFAULT_SLOGFD;

/* Currently defined logging thresholds */
static GutLogLevel sLogLevels[GUT_MAX_MODULES];
static uint32_t sNumLevels = 0;

/* ID of our logging thread. */
static pthread_t sLogThread;

/* Program name. */
static char sName[ALIAS_SIZE];

#define DEFAULT_ADDTHREADNAME    false
static bool m_addThreadName(DEFAULT_ADDTHREADNAME);

/* Program argv[0]. */
static char sProg[32];

/* Ask for logging thread shutdown. */
static bool sShutdownPending;

/* Allow for logging to be disabled completely */
static char pGutLogDisable = 0;

/* variables used for checking on config change */
static pthread_mutex_t sConfMutex = PTHREAD_MUTEX_INITIALIZER;
static time_t lastConfCheck, lastConfChange;

                            /*### @+gettid {{{2 **********************/
pid_t gettid() {
    return syscall(__NR_gettid);
}
                            /*### @-gut_tid }}}2 **********************/
                            /*### @+gut_init_logger {{{2 **********************/
void gut_init_logger(
    const char* prog,
    const char* name,
    const char* filename,
    bool addThreadName)
{
    if ( sLogFd == DEFAULT_SLOGFD ) {
        mode_t _new, old;

        /*
        ** If we're root, then open the file with write by everyone, since we
        ** know that 'nobody' will need to write to it.
        */
        if ( geteuid() == 0 )
            _new = 0;
        else
            _new = 022;

        old = umask(_new);
        if (filename != NULL && filename[0] != '\0')
            sLogFd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0666);
        else
            sLogFd = open(GUT_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0666);
        // Set the close on exec flag so nothing inherits this fd
        fcntl(sLogFd, F_SETFD, fcntl(sLogFd, F_GETFD, 0) | FD_CLOEXEC);
        umask(old);

        if ( sLogFd == DEFAULT_SLOGFD ) {
            syslog(LOG_ERR, "open(%s): %m", filename? filename:GUT_LOGFILE);
            return;
        }

        if ( sLogThread == 0 ) {
            pthread_attr_t attr;
            int err;

            /*
            ** Allocate one of the two buffers that will be used (we'll let the
            ** logging thread handle the other).
            */
            if ( !sBuf ) {
                sBuf = (GutBuf*)calloc(1, sizeof(GutBuf));
                sBuf->mCapacity = GUT_INIT_BUFSIZE;
                sBuf->mBuf = (char*)malloc(sBuf->mCapacity);

                pthread_mutex_init(&sBufMutex, NULL);
                pthread_cond_init(&sBufCond, NULL);
            }

            /*
            ** We'll run this thread detached, since there's no real opportunity
            ** to join on it.
            */
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
            if ( (err = pthread_create(&sLogThread, &attr, thread_main, NULL)) != 0 ) {
                syslog(LOG_ERR, "pthread_create(): %s", strerror(err));
                close(sLogFd);
                sLogFd = DEFAULT_SLOGFD;
                return;
            }
            pthread_attr_destroy(&attr);

            atexit(shutdown_logger);
        }
    }

    if ( prog ) {
        const char* p;

        if ( (p = strrchr(prog, '/')) )
            prog = p + 1;
    }
    else
        prog = "httpd";

    snprintf(sProg, sizeof(sProg), "%s", prog);
    if ( name )
        snprintf(sName, sizeof(sName), "%s", name);
    else
        sName[0] = '\0';

    m_addThreadName = addThreadName;
    gut_reset_logger();
}
                            /*### @-gut_init_logger }}}2 **********************/





                            /*### @+gut_clear_logger {{{2 ********************/
void gut_clear_logger_init ()
{
    // signal the gut logger thread to exit,, which in turn releases allocated
    // memory.
    shutdown_logger();

    if (DEFAULT_SLOGFD != sLogFd) {
        if (close(sLogFd) <0) {
            syslog(LOG_ERR, "close(): [%d]%s", errno, strerror(errno));
        }

        sLogFd = DEFAULT_SLOGFD; // reset to starting value
    }


    //
    // go through the globals (see top of file) and reset to startup default
    //

    memset(sLogLevels, 0, sizeof(sLogLevels));
    sNumLevels = 0;

    // sLogThread is handled by shutdown_logger

    memset(sName, 0, sizeof(sName));

    m_addThreadName = DEFAULT_ADDTHREADNAME;

    memset(sProg, 0, sizeof(sProg));

    // sShutdownPending handled by shutdown_logger

    pGutLogDisable = 0;
}
                            /*### @-gut_clear_logger }}}2 ********************/






                            /*### @+vgut_log {{{2 *****************************/
void vgut_log(const char     *file,   // name of file doing the logging __FILE__
              int            line,    // line number log is from __LINE__
              const char     *module, // module name the logging is for
              uint32_t       level,   // the gut log logging level
              const char     *fmt,    // printf style format string
              va_list        ap)      // supporting arguments for the format
{
    char lbuf[4096];
    struct timeval tv;
    struct tm ltm;
    int32_t blen, count;

    /* If logging is disabled there is nothing to do */
    if ( pGutLogDisable )
        return;

    if ( sLogFd == DEFAULT_SLOGFD )
        gut_init_logger(NULL, NULL);

    if ( sLogFd == DEFAULT_SLOGFD || sShutdownPending ||
         !log_allow(module, level) )
        return;

    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &ltm);

    count = snprintf(lbuf, sizeof(lbuf),
					 "%4d-%02d-%02d %02d:%02d:%02d.%03u [ %s(%u)",
					 ltm.tm_year + 1900, ltm.tm_mon + 1, ltm.tm_mday,
					 ltm.tm_hour, ltm.tm_min, ltm.tm_sec,
					 (uint32_t)(tv.tv_usec / 1000), sProg, (uint32_t)gettid());
    if ( count < 0 )
        return;
    blen = count;

    if ( sName[0] ) {
        count = snprintf(lbuf + blen, sizeof(lbuf) - blen, ".%s", sName);
        if ( count < 0 )
            return;
        blen += count;
    }

    if (m_addThreadName) {
        char    thread_name[24];

        // insert ??? if the get call fails
        if (0 != prctl(PR_GET_NAME, thread_name, 0, 0, 0)) {
            snprintf(thread_name, sizeof(thread_name), "???");
        }

        count = snprintf(lbuf + blen, sizeof(lbuf) - blen, ":%s", thread_name);
        if (count < 0) {
            return;
        }

        blen += count;
    }


	// File name can be:
	//    blarg.c
	//    ./CommonLib/include/blarg.h
	//    /views/jrr/......./CommonLib/include/blarg.h
	//
	//  We want just the filename, not the path
	//
	const char    *p_last_slash(strrchr(file, '/'));
	const char    *p_file;

	if (p_last_slash) {
		p_file = p_last_slash + 1;
	} else {
		p_file = file;
	}

    count = snprintf(lbuf + blen, sizeof(lbuf) - blen, " %s=%u <%s:%u> ] ",
            module, level,
            p_file, line);
    if ( count < 0 )
        return;
    blen += count;


    count = vsnprintf(lbuf + blen, (sizeof(lbuf) - blen) - 1, fmt, ap);
    
 
    if ( count < 0 )
        return;
    // check to see if output was truncated
    if ( uint32_t(count) >= ((sizeof(lbuf) - blen) - 1) )
        blen = sizeof(lbuf) - 1;
    else
        blen += count;
    // make sure string is terminated with a \n
    if ( lbuf[blen - 1] != '\n' ) {
        lbuf[blen++] = '\n';
        lbuf[blen] = '\0';
    }
    
    printf("%s", lbuf );

    pthread_mutex_lock(&sBufMutex);
    uint32_t old_capacity = sBuf->mCapacity;
    if ( sBuf->mLength + blen > sBuf->mCapacity ) {
        do {
            sBuf->mCapacity += 4096;
        }
        while ( sBuf->mCapacity < sBuf->mLength + blen );
        char* tmp = (char*)realloc(sBuf->mBuf, sBuf->mCapacity);
        // check for realloc failure
        if ( tmp != NULL )
            sBuf->mBuf = tmp;
        else {
            // restore prior info and exit
            sBuf->mCapacity = old_capacity;
            goto exit;
        }
    }
    memcpy(sBuf->mBuf + sBuf->mLength, lbuf, blen);
    sBuf->mLength += blen;
exit:
    pthread_cond_broadcast(&sBufCond);
    pthread_mutex_unlock(&sBufMutex);
}
                            /*### @-vgut_log }}}2 ******************************/
                            /*### @+gut_log {{{2 ******************************/
void gut_log(const char     *file,   // name of file doing the logging __FILE__
             int            line,    // line number log is from __LINE__
             const char     *module, // module name the logging is for
             uint32_t       level,   // the gut log logging level
             const char     *fmt,    // printf style format string
             ...)                    // supporting arguments for the format
{
    va_list ap;

    va_start(ap, fmt);
    vgut_log(file, line, module, level, fmt, ap);
    va_end(ap);
}
                            /*### @-gut_log }}}2 ******************************/
                            /*### @+gut_reset_logger {{{2 *********************/
void gut_reset_logger()
{
    const char* logConfig;
    char lbuf[1024];
    char* module;
    char* slevel;
    char* pos;
    uint32_t level;
    uint32_t i;

    // set config change varaibles;
    time(&lastConfCheck);
    lastConfChange = lastConfCheck;

    memset(sLogLevels, 0, sizeof(sLogLevels));
    /* The 0th entry is always the default */
    sLogLevels[0].mLevel = 1;
    sNumLevels = 1;

    snprintf(lbuf, sizeof(lbuf), "%s.%s_loglevel", sProg, sName);

    if ( !(logConfig = gut_get_user_config_value(lbuf)) ) {
        snprintf(lbuf, sizeof(lbuf), "%s_loglevel", sProg);
        logConfig = gut_get_user_config_value(lbuf);
    }

    if ( !(logConfig = gut_get_user_config_value(lbuf)) ) {
        snprintf(lbuf, sizeof(lbuf), "loglevel");
        logConfig = gut_get_user_config_value(lbuf);
    }

    if ( logConfig && *logConfig ) {
        snprintf(lbuf, sizeof(lbuf), "%s", logConfig);
        /* backward compatibility */
        if ( strcmp(lbuf, "none") == 0 )
            strcpy(lbuf, "DEFAULT=0");
        else if ( strcmp(lbuf, "critical") == 0 )
            strcpy(lbuf, "DEFAULT=1");
        else if ( strcmp(lbuf, "debug") == 0 )
            strcpy(lbuf, "DEFAULT=3");
        else if ( strcmp(lbuf, "trace") == 0 )
            strcpy(lbuf, "DEFAULT=10");

        module = strtok_r(lbuf, ",= \n", &pos);
        while ( module && sNumLevels < GUT_MAX_MODULES ) {
            if ( !(slevel = strtok_r(NULL, ",= \n", &pos)) ) {
                syslog(LOG_ERR, "Syntax error in log configuration entry (\"%s\"): missing level", logConfig);
                break;
            }
            if ( (level = strtoul(slevel, NULL, 10)) == 0 )
                syslog(LOG_ERR, "Warning: logging disabled for module \"%s.%s\"", sProg, module);

            if ( strcmp(module, "DEFAULT") == 0 )
                sLogLevels[0].mLevel = level;
            else {
                GutLogLevel* l;
                for ( i = 1; i < sNumLevels; ++i ) {
                    if ( strcmp(module, sLogLevels[i].mModule) == 0 ) {
                        sLogLevels[i].mLevel = level;
                        break;
                    }
                }
                if ( i == sNumLevels ) {
                    l = &sLogLevels[sNumLevels++];
                    snprintf(l->mModule, sizeof(l->mModule), "%s", module);
                    l->mLevel = level;
                }
            }

            module = strtok_r(NULL, ",= \n", &pos);
        }

        if ( sNumLevels == GUT_MAX_MODULES ) {
            syslog(LOG_ERR, "Warning: too many modules configured for logging (max %u)",
                    GUT_MAX_MODULES);
        }

        if ( sNumLevels > 2 )
            qsort(sLogLevels + 1, sNumLevels - 1, sizeof(GutLogLevel), llcompare);
    }
}
                            /*### @-gut_reset_logger }}}2 *********************/
                            /*### @+shutdown_logger {{{2 **********************/

/*
** Tell our logging thread to shutdown and wait for it.
*/
static void shutdown_logger()
{
    if ( sLogThread != 0 ) {
        pthread_mutex_lock(&sBufMutex);
        sShutdownPending = true;
        pthread_cond_broadcast(&sBufCond);
        pthread_mutex_unlock(&sBufMutex);

        pthread_join(sLogThread, NULL);
        sLogThread = 0;
        sShutdownPending = false;
    }
}
                            /*### @-shutdown_logger }}}2 **********************/
                            /*### @+llcompare {{{2 ****************************/

/*
** qsort comparison function for log level entries.
*/
static int llcompare(
    const void* vlhs,
    const void* vrhs)
{
    const GutLogLevel* lhs = (const GutLogLevel*)vlhs;
    const GutLogLevel* rhs = (const GutLogLevel*)vrhs;

    return strcmp(lhs->mModule, rhs->mModule);
}
                            /*### @-llcompare }}}2 ****************************/
                            /*### @+log_allow {{{2 ****************************/

/*
** Called by loggers to see if a particular message should be logged.
*/
static bool log_allow(
    const char* module,
    uint32_t level)
{
    GutLogLevel* l;
    uint32_t i;
    int cmp;
    time_t now;
    struct stat buf;


    // check config changes and reload if required
    time(&now);

    if ( now - lastConfCheck > 2*60 ) {
        pthread_mutex_lock(&sConfMutex);
        if ( stat(gut_get_user_config_filename(), &buf) == 0 ) {
            if ( lastConfChange < buf.st_mtime ) {
                gut_refresh_user_config();
                gut_reset_logger();
                lastConfChange = buf.st_mtime;
            }
        }
        lastConfCheck = now;
        pthread_mutex_unlock(&sConfMutex);
    }

    for ( i = 1; i < sNumLevels; ++ i ) {
        l = &sLogLevels[i];
        if ( (cmp = strcmp(l->mModule, module)) == 0 ) {
            if ( l->mLevel >= level )
                return true;
            else
                return false;
        }
        else if ( cmp > 0 )
            break;
    }

    /* no entry for the module: use the default */
    if ( sLogLevels[0].mLevel >= level )
        return true;
    else
        return false;
}
                            /*### @-log_allow }}}2 ****************************/
                            /*### @-gut_log_disable }}}2 **********************/
void gut_log_disable()
{
    pGutLogDisable = 1;
}
                            /*### @-gut_log_disable }}}2 **********************/
                            /*### @-gut_log_enable }}}2 ***********************/
void gut_log_enable()
{
    pGutLogDisable = 0;
}
                            /*### @-gut_log_enable }}}2 ***********************/
                            /*### @+thread_main {{{2 **************************/

/*
** Main program for logging thread.
*/
static void* thread_main(void*)
{
    sigset_t blockedSigs;
    struct timespec ts;
    GutBuf* alt;
    GutBuf* tmp;

    /* block all signals for this thread */
    sigfillset(&blockedSigs);
    pthread_sigmask(SIG_SETMASK, &blockedSigs, NULL);

    alt = (GutBuf*)calloc(1, sizeof(GutBuf));
    alt->mCapacity = GUT_INIT_BUFSIZE;
    alt->mBuf = (char*)malloc(alt->mCapacity);

    // Set the name to help identify this via proc
    prctl(PR_SET_NAME,"gut_log",0,0,0);

    for ( ;; ) {
        pthread_mutex_lock(&sBufMutex);

        while ( sBuf->mLength == 0 && !sShutdownPending )
            pthread_cond_wait(&sBufCond, &sBufMutex);

        tmp = sBuf;
        sBuf = alt;
        alt = tmp;
        pthread_mutex_unlock(&sBufMutex);

        if ( alt->mLength > 0 ) {
            write(sLogFd, alt->mBuf, alt->mLength);
            alt->mLength = 0;
            if ( !sShutdownPending ) {
                if ( alt->mCapacity > GUT_MAX_BUFSIZE ) {
                    alt->mCapacity = GUT_MAX_BUFSIZE;
                    alt->mBuf = (char*)realloc(alt->mBuf, alt->mCapacity);
                }

                /*
                ** If we don't have another message in the queue, sleep for a
                ** little bit.  The purpose of this is to both relinquish CPU
                ** control and to increase the possibility of having multiple
                ** messages get bundled into the same write.
                */
                pthread_mutex_lock(&sBufMutex);
                uint32_t    sBufLength(sBuf->mLength);
                pthread_mutex_unlock(&sBufMutex);

                if ( !sBufLength ) {
                    ts.tv_sec = 0;
                    ts.tv_nsec = 40000000; /* 40ms */
                    nanosleep(&ts, NULL);
                }
            }
        }

        if (sShutdownPending) {
            break;
        }
    }

    // we're shutting down, no more entries allowed
    pGutLogDisable = 1;


    // release all of the memory allocated earlier. It's not strictly needed,
    // but it removes a ton of valgrind memory leak errors.
    if (sBuf) {
        free(sBuf->mBuf);
        free(sBuf);
        sBuf = NULL;
    }

    if (alt) {
        free(alt->mBuf);
        free(alt);
        alt = NULL;
    }

    gut_refresh_user_config(); // release any user config allocated memory
    pthread_exit(NULL);
}
                            /*### @-thread_main }}}2 **************************/


/* ==================== editors ====================== */

