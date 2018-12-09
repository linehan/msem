#ifndef _JLIB_SHELL_H
#define _JLIB_SHELL_H
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>

extern int shell(const char *fmt, ...);
extern int bounce(char *buf, size_t max, const char *fmt, ...);
extern int mecho(const char *fmt, ...);
extern int pipe_status(void);

#ifndef PIPE_NONE
#define PIPE_NONE   0
#else
#error "PIPE_NONE defined elsewhere"
#endif

#ifndef PIPE_LINE
#define PIPE_LINE   1 
#else
#error "PIPE_LINE defined elsewhere"
#endif

#ifndef PIPE_INPUT
#define PIPE_INPUT  2 
#else
#error "PIPE_INPUT defined elsewhere"
#endif

#ifndef PIPE_OUTPUT
#define PIPE_OUTPUT 3 
#else
#error "PIPE_OUTPUT defined elsewhere"
#endif

extern bool WAITING_INPUT;
extern bool WAITING_OUTPUT;

#ifndef WAITING
#define WAITING(stream) (!(isatty(fileno(stream))))
#else
#error "WAITING defined elsewhere"
#endif

#endif
