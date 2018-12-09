
#ifndef _DEBUG_H
#define _DEBUG_H
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include "util.h"

/*
 * OPTION MACROS
 * `````````````
 * #define any of the following *BEFORE* including 
 * this header to alter certain behaviors.
 *
 * _JDL_NO_PRINT
 * Suppress all print macros. 
 *
 * _JDL_NO_PRINT_LOCATION
 * Suppress all location information.
 *
 * _JDL_NO_PRINT_DEBUG
 * Suppress DEBUG macro.
 *
 * _JDL_NO_PRINT_DEBUG_LOCATION
 * Suppress location information for DEBUG macro.
 *
 * _JDL_NO_PRINT_ERROR
 * Suppress ERROR macro.
 *
 * _JDL_NO_PRINT_ERROR_LOCATION
 * Suppress location information for ERROR macro.
 *
 * _JDL_NO_PRINT_WARN
 * Suppress WARN macro.
 *
 * _JDL_NO_PRINT_WARN_LOCATION
 * Suppress location information for WARN macro.
 *
 * _JDL_NO_PRINT_PRINT
 * Suppress PRINT macro.
 *
 * _JDL_NO_PRINT_PRINT_LOCATION
 * Suppress location information for PRINT macro.
 *
 * _JDL_NO_PRINT_TRACE
 * Suppress __ENTER, __LEAVE, and __BREAK macros.
 *
 * _JDL_NO_HALT_ON_PANIC
 * Causes PANIC to print with ERROR, but not halt
 * the program.
 */


#define WHERE_FMT  "%s:%d: "
#define WHERE_ARG  __FILE__, __LINE__

/* With location information */
#define MULTIPRINT_LOC_1(...)      fprintf(stderr, WHERE_FMT __VA_ARGS__, WHERE_ARG)
#define MULTIPRINT_LOC_2(fmt, ...) fprintf(stderr, WHERE_FMT fmt, WHERE_ARG, __VA_ARGS__)
#define MULTIPRINT_LOC_3           MULTIPRINT_LOC_2
#define MULTIPRINT_LOC_4           MULTIPRINT_LOC_2
#define MULTIPRINT_LOC_5           MULTIPRINT_LOC_2
#define MULTIPRINT_LOC_6           MULTIPRINT_LOC_2
#define MULTIPRINT_LOC_7           MULTIPRINT_LOC_2
#define MULTIPRINT_LOC_8           MULTIPRINT_LOC_2
#define MULTIPRINT_LOC_9           MULTIPRINT_LOC_2

#define PRINT_STRINGS_LOC(N, ...)  MULTIPRINT_LOC_ ## N(__VA_ARGS__) // N ->  1
#define COUNT_STRINGS_LOC(N, ...)  PRINT_STRINGS_LOC(N, __VA_ARGS__) // N -> (1)
#define MULTIPRINT_LOC(...)        COUNT_STRINGS_LOC(NUM_ARGS(__VA_ARGS__), __VA_ARGS__)

/* Without location information */
#define MULTIPRINT_1(...)          fprintf(stderr, __VA_ARGS__)
#define MULTIPRINT_2(fmt, ...)     fprintf(stderr, fmt, __VA_ARGS__)
#define MULTIPRINT_3               MULTIPRINT_2
#define MULTIPRINT_4               MULTIPRINT_2
#define MULTIPRINT_5               MULTIPRINT_2
#define MULTIPRINT_6               MULTIPRINT_2
#define MULTIPRINT_7               MULTIPRINT_2
#define MULTIPRINT_8               MULTIPRINT_2
#define MULTIPRINT_9               MULTIPRINT_2

#define PRINT_STRINGS(N, ...)      MULTIPRINT_ ## N(__VA_ARGS__) // N ->  1
#define COUNT_STRINGS(N, ...)      PRINT_STRINGS(N, __VA_ARGS__) // N -> (1)
#define MULTIPRINT(...)            COUNT_STRINGS(NUM_ARGS(__VA_ARGS__), __VA_ARGS__)



/*
 * Print a debug message 
 */
#if (defined _JDL_NO_PRINT_DEBUG || defined _JDL_NO_PRINT)
#define DEBUG(...)  /* nothing */
#elif (defined _JDL_NO_PRINT_DEBUG_LOCATION || defined _JDL_NO_PRINT_LOCATION)
#define DEBUG(...)  MULTIPRINT("DEBUG: " __VA_ARGS__)
#else
#define DEBUG(...)  MULTIPRINT_LOC("DEBUG: " __VA_ARGS__)
#endif


/*
 * Print an error message.
 */
#if (defined _JDL_NO_PRINT_ERROR || defined _JDL_NO_PRINT)
#define ERROR(...)  /* nothing */
#elif (defined _JDL_NO_PRINT_ERROR_LOCATION || defined _JDL_NO_PRINT_LOCATION)
#define ERROR(...)  MULTIPRINT("ERROR: " __VA_ARGS__)
#else
#define ERROR(...)  MULTIPRINT_LOC("ERROR: " __VA_ARGS__)
#endif


/* 
 * Print a warning message.
 */
#if (defined _JDL_NO_PRINT_WARN || defined _JDL_NO_PRINT)
#define WARN(...)  /* nothing */
#elif (defined _JDL_NO_PRINT_WARN_LOCATION || defined _JDL_NO_PRINT_LOCATION)
#define WARN(...)  MULTIPRINT("WARN: " __VA_ARGS__)
#else
#define WARN(...)  MULTIPRINT_LOC("WARN: " __VA_ARGS__)
#endif


/* 
 * Print a normal message.
 */
#if (defined _JDL_NO_PRINT_PRINT || defined _JDL_NO_PRINT)
#define PRINT(...)  /* nothing */
#elif (defined _JDL_NO_PRINT_PRINT_LOCATION || defined _JDL_NO_PRINT_LOCATION)
#define PRINT(...)  MULTIPRINT(__VA_ARGS__)
#else
#define PRINT(...)  MULTIPRINT_LOC(" " __VA_ARGS__)
#endif

#define P PRINT


/*
 * Print an error and halt program execution.
 */
#if (defined _JDL_NO_PRINT)
#define PANIC(...) /* nothing */
#elif (defined _JDL_NO_HALT_ON_PANIC)
#define PANIC(...) ERROR(__VA_ARGS__) 
#else
#define PANIC(...)                                          \
        do {                                                \
                ERROR(__VA_ARGS__);                         \
                halt(SIGABRT, "Internal error.");           \
        } while (0)
#endif


/*
 * Print trace messages.
 */
#if (defined _JDL_NO_PRINT_TRACE || defined _JDL_NO_PRINT)
#define __ENTER         /* nothing */
#define __LEAVE         /* nothing */
#define __BREAK         /* nothing */
#else
#define __ENTER         fprintf(stderr, "Entered %s\n", __func__)
#define __LEAVE         fprintf(stderr, "Leaving %s\n", __func__)
#define __BREAK         PANIC("Hit breakpoint")
#endif



#define assert_msg(test, msg) assert(test && msg);

/* e.g. 
        when(x) {
                printf("assertion will fail\n"); 
        }
 */
#define when(x) for (; (x); assert(x))


/*
 * Exit the program and print a diagnostic message
 */
#define bye(...)                                                         \
        (NUM_ARGS(__VA_ARGS__) == 1) ? abort_report(__VA_ARGS__, "")  \
                                     : abort_report(__VA_ARGS__)

/*
 * Raise a signal and print an error.
 */

#define halt(sig, ...)                                                    \
        (NUM_ARGS(__VA_ARGS__) == 1) ? raise_report(sig, __VA_ARGS__, "") \
                                     : raise_report(sig, __VA_ARGS__)


extern int abort_report(const char *fmt, ...);
extern int debug_report(const char *fmt, ...);
extern int debug_raise_report(int signo, const char *fmt, ...);
extern int raise_report(int signo, const char *fmt, ...);
extern int internal_error(int signo, const char *fmt, ...);
extern void file_watch(char *path, char *usr_args, bool loop);
extern void file_follow(char *path, bool loop);

int set_errno(int number);


#endif

