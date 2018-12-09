#ifndef _JLIB_FORMAT_STRING_H
#define _JLIB_FORMAT_STRING_H
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/**
 * FORMAT_WITH
 * ```````````
 * Cleanly write a format string and arguments into a buffer.
 *
 * @fmt_var: The name of the format string variable
 * @buf_var: The name of the buffer variable
 * @buf_siz: The size of the buffer
 *
 * NOTE
 * This macro is only a rug to sweep things under.
 * Functions which accept format arguments are less 
 * confusing without all of the format-handling code
 * cluttering the top.
 *
 * USAGE
 *      int open_fmt(const char *fmt, ...)
 *      {
 *              static char path[PATHSIZE];
 *              int fd;
 *
 *              FORMAT_WITH(fmt, path, PATHSIZE);
 *
 *              fd = open(path, O_RDWR, 0666);
 *
 *              return fd;
 *      }
 */
#define FORMAT_WITH(fmt_var, buf_var, buf_siz)                  \
        va_list _JLIB_FMT_ARG;                                  \
                                                                \
        va_start(_JLIB_FMT_ARG, fmt_var);                       \
        vsnprintf(buf_var, buf_siz, fmt_var, _JLIB_FMT_ARG);    \
        va_end(_JLIB_FMT_ARG)                                   




#define FPRINTF_WITH(fmt_var, file_stream)                      \
        va_list _JLIB_FMT_LIST;                                 \
                                                                \
        va_start(_JLIB_FMT_LIST, fmt_var);                      \
        vfprintf(file_stream, fmt_var, _JLIB_FMT_LIST);         \
        va_end(_JLIB_FMT_LIST)                                   


#endif
